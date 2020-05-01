
#include <objrepr/reprServer.h>

#include "from_ms_common/common/ms_common_utils.h"
#include "from_ms_common/communication/network_interface.h"
#include "from_ms_common/system/logger.h"
#include "from_ms_common/system/object_pool.h"
#include "commands/cmd_ping.h"
#include "common_vars.h"
#include "video_server_handler.h"

static constexpr int64_t PING_INTERVAL_MILLISEC = 1000;
static constexpr int64_t SERVER_TIMEOUT_MILLISEC = 10000;

namespace video_server_client{

using namespace std;
using namespace common_types;


// --------------------------------------------------------------------
// private
// --------------------------------------------------------------------
class PrivateImplementationVSH : public INetworkObserver, public ICommandCallbacksObserver {
public:
    PrivateImplementationVSH()
        : interface(nullptr)
        , online(false)
        , lastPongMillisec(0)
        , lastPingAtMillisec(0)
        , archiversUpdate(false)
        , analyzersUpdate(false)
    {}

    virtual void pongCatched() override {
        lastPongMillisec = common_utils::getCurrentTimeMillisec();
    }

    virtual void archiversInUpdate( bool _update ) override {
        archiversUpdate = _update;
    }

    virtual void analyzersInUpdate( bool _update ) override {
        analyzersUpdate = _update;
    }

    virtual void archiverIsReady( const TArchivingId & _archId ) override {

        for( auto & valuePair : archivingHandlersBySensorId ){
            PArchiveHandler & archiveHandler = valuePair.second;

            if( archiveHandler->getArchiveStatus().archivingId == _archId ){
                archivingHandlersByArchivingId.insert( {_archId, archiveHandler} );
            }
        }
    }

    virtual void analyzerIsReady( const TProcessingId & _procId ) override {

        for( auto & valuePair : analyzeHandlersBySensorId ){
            PAnalyzeHandler & analyzeHandler = valuePair.second;

            if( analyzeHandler->getAnalyzeStatus().processingId == _procId ){
                analyzeHandlersByProcessingId.insert( {_procId, analyzeHandler} );
            }
        }
    }

    virtual void updateServerState( const SServerState & _state ) override {
        // TODO: assign by one variable
//        status->objreprId = _state.objreprId;
        status->currentContextId = _state.currentContextId;
        status->role = _state.role;
    }

    virtual void updateSystemState( const SSystemState & _state ) override {
        // TODO: assign by one variable
        status->cpuUsagePercent = _state.cpuUsagePercent;
        status->systemMemoryUsagePercent = _state.systemMemoryUsagePercent;
        status->gpuUsagePercent = _state.gpuUsagePercent;
        status->gpuMemoryUsagePercent = _state.gpuMemoryUsagePercent;
    }

    virtual void updateArchivingStatus( const std::vector<SArchiveStatus> & _status ) override {

        vector<string> disappeared;
        for( const SArchiveStatus & state : _status ){
            archiveHandlersLock.lock();
            auto iter = archivingHandlersByArchivingId.find( state.archivingId );
            if( iter != archivingHandlersByArchivingId.end() ){
                PArchiveHandler archiverHandler = iter->second;
                archiverHandler->addStatus( state, false );
            }
            else{
                if( ! archiversUpdate ){
                    disappeared.push_back( state.archivingId );
                }
                else{
                    PArchiveHandler archiveHandler = std::make_shared<ArchiveHandler>( commandServices );
                    archiveHandler->addStatus( state, false );
                    archivingHandlersBySensorId.insert( {state.sensorId, archiveHandler} );
                    archivingHandlersByArchivingId.insert( {state.archivingId, archiveHandler} );
                }
            }
            archiveHandlersLock.unlock();
        }

        if( ! disappeared.empty() ){
//            VS_LOG_WARN << PRINT_HEADER << " ------------------- there is a disappeared archives -------------------" << endl;
        }
    }

    virtual void updateAnalyzeStatus( const std::vector<SAnalyzeStatus> & _status ) override {

        vector<string> disappeared;
        for( const SAnalyzeStatus & state : _status ){
            analyzeHandlersLock.lock();
            auto iter = analyzeHandlersByProcessingId.find( state.processingId );
            if( iter != analyzeHandlersByProcessingId.end() ){
                PAnalyzeHandler analyzeHandler = iter->second;
                analyzeHandler->addStatus( state, false );
            }
            else{
                if( ! analyzersUpdate ){
                    disappeared.push_back( state.processingId );
                }
                else{
                    PAnalyzeHandler analyzeHandler = std::make_shared<AnalyzeHandler>( commandServices );
                    analyzeHandler->addStatus( state, false );
                    analyzeHandlersBySensorId.insert( {state.sensorId, analyzeHandler} );
                    analyzeHandlersByProcessingId.insert( {state.processingId, analyzeHandler} );
                }
            }
            analyzeHandlersLock.unlock();
        }

        if( ! disappeared.empty() ){
//            VS_LOG_WARN << PRINT_HEADER << " ------------------- there is a disappeared analyzers -------------------" << endl;
        }
    }

    virtual void newEvent( std::vector<SAnalyticEvent> && _event ) override {

        for( SAnalyticEvent & event : _event ){
            analyzeHandlersLock.lock();
            auto iter = analyzeHandlersByProcessingId.find( event.m_processingId );
            if( iter != analyzeHandlersByProcessingId.end() ){
                PAnalyzeHandler analyzeHandler = iter->second;

                PAnalyticEvent eventPtr = poolOfExternalEvents.getInstance();
                eventPtr->m_sensorId = event.m_sensorId;
                eventPtr->m_processingId = event.m_processingId;
                eventPtr->m_totalCount = event.m_totalCount;
                eventPtr->m_newObjects.swap( event.m_newObjects );
                eventPtr->m_changedObjects.swap( event.m_changedObjects );
                eventPtr->m_disapprearedObjects.swap( event.m_disapprearedObjects );

                analyzeHandler->addEvent( eventPtr );
            }
            analyzeHandlersLock.unlock();
        }
    }

    virtual void addCommmand( PCommand _cmd ) override {

        commandsInProgress.push_back( _cmd );
    }

    virtual void addFuture( std::future<void> && _future ) override {

        futureForDeferredSignal.push_back( std::move(_future) );
    }

    virtual void callbackNetworkRequest( PEnvironmentRequest _request ) override {

        VS_LOG_INFO << PRINT_HEADER
                    << " this method highly likely is not necessary"
                    << " , but msg [" << _request->m_incomingMessage << "]"
                    << endl;
    }

    void updateArchivingHandlers(){

        archiveHandlersLock.lock();
        archivingHandlersBySensorId.clear();
        archivingHandlersByArchivingId.clear();
        archiveHandlersLock.unlock();

        CommandArchivingStatus::SInitialParams params;
        params.archivingId = CommandArchivingStatus::ALL_ARCHIVINGS;

        PCommandArchivingStatus cmd = std::make_shared<CommandArchivingStatus>( & commandServices );
        cmd->m_params = params;
        cmd->execAsync();
        commandsInProgress.push_back( cmd );
    }

    void disableArchivingHandlers(){

        archiveHandlersLock.lock();
        for( auto & valuePair : archivingHandlersBySensorId ){
            PArchiveHandler handler = valuePair.second;
            SArchiveStatus as;
            as.archiveState = EArchiveState::UNAVAILABLE; // TODO: make more elegant
            handler->addStatus( as, false );
            handler->sendSignalStateChanged( EArchiveState::UNAVAILABLE );
        }
        // TODO: delete archiver ?
        archiveHandlersLock.unlock();
    }

    void updateAnalyzeHandlers(){

        analyzeHandlersLock.lock();
        analyzeHandlersBySensorId.clear();
        analyzeHandlersByProcessingId.clear();
        analyzeHandlersLock.unlock();

        CommandAnalyzeStatus::SInitialParams params;
        params.sensorId = CommandAnalyzeStatus::ALL_SENSORS;
        params.processingId = CommandAnalyzeStatus::ALL_PROCESSING;

        PCommandAnalyzeStatus cmd = std::make_shared<CommandAnalyzeStatus>( & commandServices );
        cmd->m_params = params;
        cmd->execAsync();
        commandsInProgress.push_back( cmd );
    }

    void disableAnalyzeHandlers(){

        analyzeHandlersLock.lock();
        for( auto & valuePair : analyzeHandlersBySensorId ){
            PAnalyzeHandler handler = valuePair.second;
            SAnalyzeStatus as;
            as.analyzeState = EAnalyzeState::UNAVAILABLE; // TODO: make more elegant
            handler->addStatus( as, false );
            handler->sendSignalStateChanged( EAnalyzeState::UNAVAILABLE );
        }
        // TODO: delete analyzer
        analyzeHandlersLock.unlock();
    }

    void runSystemClock(){

        networkProvider->runNetworkCallbacks();
        ping();
        remoteServicesMonitoring();
        commandsProcessing();
        clearFutures();
    }

    inline void commandsProcessing(){

        for( auto iter = commandsInProgress.begin(); iter != commandsInProgress.end(); ){
            PCommand & cmd = ( * iter );

            if( cmd->isReady() ){
                iter = commandsInProgress.erase( iter );
            }
            else{
                ++iter;
            }
        }
    }

    inline void remoteServicesMonitoring(){

        if( online ){
            if( (common_utils::getCurrentTimeMillisec() - lastPongMillisec) > SERVER_TIMEOUT_MILLISEC ){
                VS_LOG_INFO << PRINT_HEADER << " VideoServer OFFLINE at " << common_utils::getCurrentDateTimeStr() << endl;
                disableArchivingHandlers();
                disableAnalyzeHandlers();
                online = false;
                status->connectionEstablished = false;
                interface->sendSignalOnline( false );
            }
        }
        else{
            if( (common_utils::getCurrentTimeMillisec() - lastPongMillisec) < SERVER_TIMEOUT_MILLISEC ){
                VS_LOG_INFO << PRINT_HEADER << " VideoServer ONLINE at " << common_utils::getCurrentDateTimeStr() << endl;
                updateArchivingHandlers();
                updateAnalyzeHandlers();
                online = true;
                status->connectionEstablished = true;
                interface->sendSignalOnline( true );
            }
        }
    }

    inline void ping(){

        if( (common_utils::getCurrentTimeMillisec() - lastPingAtMillisec) > PING_INTERVAL_MILLISEC ){
            if( commandPing->isReady() ){
                commandPing->execAsync();
                commandsInProgress.push_back( commandPing );
            }

            lastPingAtMillisec = common_utils::getCurrentTimeMillisec();
        }
    }

    inline void clearFutures(){

        for( auto iter = futureForDeferredSignal.begin(); iter != futureForDeferredSignal.end(); ){
            std::future<void> & analyzeFuture = ( * iter );

            const std::future_status futureStatus = analyzeFuture.wait_for( std::chrono::milliseconds(10) );
            if( std::future_status::ready == futureStatus ){
                iter = futureForDeferredSignal.erase( iter );
            }
            else{
                ++iter;
            }
        }
    }

    bool isServerAvailable( bool * _errOccured, std::string * _errMsg ){

        if( ! status->connectionEstablished ){
            stringstream ss;
            ss << PRINT_HEADER << " connection to server is NOT established";
            lastError = ss.str();
            cout << ss.str() << endl;
            if( _errOccured ){
                ( * _errOccured ) = true;
            }
            if( _errMsg ){
                ( * _errMsg ) = ss.str();
            }
            return false;
        }

        if( objrepr::RepresentationServer::instance()->currentContext()->id() != status->currentContextId ){
            stringstream ss;
            ss << PRINT_HEADER << " connection to server with another opened context ["
               << getContextName( status->currentContextId ) << "]";
            lastError = ss.str();
            cout << ss.str() << endl;
            if( _errOccured ){
                ( * _errOccured ) = true;
            }
            if( _errMsg ){
                ( * _errMsg ) = ss.str();
            }
            return false;
        }

        return true;
    }

    string getContextName( const uint32_t _ctxId ){
        for( const objrepr::ContextPtr ctx : objrepr::RepresentationServer::instance()->contextList() ){
            if( ctx->id() == _ctxId ){
                return ctx->name();
            }
        }
        return string();
    }

    // data
    PCommandPlayerPing commandPing;
    PVideoServerStatus status;
    int64_t lastPongMillisec;
    int64_t lastPingAtMillisec;
    bool online;
    VideoServerHandler::SInitSettings settings;
    std::string lastError;
    std::vector<PCommand> commandsInProgress;
    std::unordered_multimap<TSensorId, PAnalyzeHandler> analyzeHandlersBySensorId;
    std::unordered_map<TProcessingId, PAnalyzeHandler> analyzeHandlersByProcessingId;
    std::unordered_multimap<TSensorId, PArchiveHandler> archivingHandlersBySensorId;
    std::unordered_map<TArchivingId, PArchiveHandler> archivingHandlersByArchivingId;
    bool archiversUpdate;
    bool analyzersUpdate;
    std::vector<std::future<void>> futureForDeferredSignal;

    // service
    VideoServerHandler * interface;
    PNetworkProvider networkProvider;
    SCommandServices commandServices;
    std::mutex muCommandsInProgress;
    std::mutex analyzeHandlersLock;
    std::mutex archiveHandlersLock;
    ObjectPool<SAnalyticEvent> poolOfExternalEvents;
};


// --------------------------------------------------------------------
// public
// --------------------------------------------------------------------
VideoServerHandler::VideoServerHandler()
    : m_impl(new PrivateImplementationVSH())
{
    m_impl->interface = this;
}

VideoServerHandler::~VideoServerHandler(){

    delete m_impl;
    m_impl = nullptr;
}

void VideoServerHandler::runSystemClock(){

    m_impl->runSystemClock();
}

bool VideoServerHandler::init( const SInitSettings & _settings ){

    m_impl->settings = _settings;

    // command services
    m_impl->commandServices.networkClient = _settings.networkClient;
    m_impl->commandServices.callbacks = m_impl;

    // status
    m_impl->status = std::make_shared<SVideoServerStatus>();
    m_impl->status->objreprId = _settings.objreprId;
    m_impl->status->role = _settings.role;

    // network
    PNetworkProvider netProvider = std::dynamic_pointer_cast<INetworkProvider>( _settings.networkClient );
    netProvider->addObserver( m_impl );
    m_impl->networkProvider = netProvider;

    // pind
    m_impl->commandPing = std::make_shared<CommandPlayerPing>( & m_impl->commandServices );


    return true;
}

SVideoServerStatus & VideoServerHandler::getStatusRef(){

    return ( * m_impl->status );
}

const std::string & VideoServerHandler::getLastError(){

    return m_impl->lastError;
}

PConstVideoServerStatus VideoServerHandler::getStatus(){
//    VS_LOG_INFO << PRINT_HEADER << " 'get status' called, conn: "
//                << m_impl->status->connectionEstablished
//                << " obj id: " << m_impl->status->objreprId
//                << endl;
    return m_impl->status;
}

// analyze
void VideoServerHandler::destroyAnalyze( PAnalyzeHandler _handler ){

    m_impl->analyzeHandlersLock.lock();
    auto iter = m_impl->analyzeHandlersByProcessingId.find( _handler->getAnalyzeStatus().processingId );
    if( iter != m_impl->analyzeHandlersByProcessingId.end() ){

        VS_LOG_INFO << PRINT_HEADER
                    << " stop & destroy analyze handler on procId [" << _handler->getInitialParams().sensorId << "]"
                    << endl;

        PAnalyzeHandler analyzeHandler = iter->second;

        m_impl->analyzeHandlersLock.unlock();
        analyzeHandler->stop( true );
        m_impl->analyzeHandlersLock.lock();

        m_impl->analyzeHandlersBySensorId.erase( analyzeHandler->getInitialParams().sensorId );
        m_impl->analyzeHandlersByProcessingId.erase( iter );
    }
    m_impl->analyzeHandlersLock.unlock();
}

std::unordered_multimap<TSensorId, PAnalyzeHandler> & VideoServerHandler::getAnalyzeHandlers(){
    return m_impl->analyzeHandlersBySensorId;
}

std::unordered_map<TProcessingId, PAnalyzeHandler> & VideoServerHandler::getAnalyzeHandlers2(){
    return m_impl->analyzeHandlersByProcessingId;
}

PAnalyzeHandler VideoServerHandler::launchAnalyze( CommandAnalyzeStart::SInitialParams _params,
        bool * _errOccured,
        std::string * _errMsg ){

    // check
    if( ! m_impl->isServerAvailable(_errOccured, _errMsg) ){
        return nullptr;
    }

    // init
    PAnalyzeHandler analyzeHandler = std::make_shared<AnalyzeHandler>( m_impl->commandServices );
    if( ! analyzeHandler->init(_params) ){
        stringstream ss;
        ss << PRINT_HEADER << " analyzer init ERROR [" << analyzeHandler->getLastError() << "]";
        m_impl->lastError = ss.str();
        VS_LOG_ERROR << ss.str() << endl;
        if( _errMsg ){
            ( * _errMsg ) = ss.str();
        }
        if( _errOccured ){
            ( * _errOccured ) = true;
        }
        return nullptr;
    }

    // start
    analyzeHandler->start();

    m_impl->analyzeHandlersLock.lock();
    m_impl->analyzeHandlersBySensorId.insert( {_params.sensorId, analyzeHandler} );
    m_impl->analyzeHandlersLock.unlock();

    if( _errOccured ){
        ( * _errOccured ) = false;
    }
    return analyzeHandler;
}

// retranslation
bool VideoServerHandler::sourceConnect( CommandConnectSource::SInitialParams _params ){

    // TODO: do
    return false;
}

bool VideoServerHandler::sourceDisconnect( CommandConnectSource::SInitialParams _params ){

    // TODO: do
    return false;
}

// archive
void VideoServerHandler::destroyArchiving( PArchiveHandler _handler ){

    m_impl->archiveHandlersLock.lock();
    auto iter = m_impl->archivingHandlersBySensorId.find( _handler->getInitialParams().sensorId );
    if( iter != m_impl->archivingHandlersBySensorId.end() ){

        VS_LOG_INFO << PRINT_HEADER
                    << " stop & destroy archive handler on archId [" << _handler->getInitialParams().archivingId << "]"
                    << endl;

        PArchiveHandler archiveHandler = iter->second;

        m_impl->archiveHandlersLock.unlock();
        archiveHandler->stop( true );
        m_impl->archiveHandlersLock.lock();

        m_impl->archivingHandlersBySensorId.erase( iter->first );
        m_impl->archivingHandlersByArchivingId.erase( _handler->getInitialParams().archivingId );
    }
    m_impl->archiveHandlersLock.unlock();
}

std::unordered_multimap<TSensorId, PArchiveHandler> & VideoServerHandler::getArchiveHandlers(){
    return m_impl->archivingHandlersBySensorId;
}

std::unordered_map<TArchivingId, PArchiveHandler> & VideoServerHandler::getArchiveHandlers2(){
    return m_impl->archivingHandlersByArchivingId;
}

PArchiveHandler VideoServerHandler::launchArchiving( CommandArchiveStart::SInitialParams _params,
        bool * _errOccured,
        std::string * _errMsg ){

    // check
    if( ! m_impl->isServerAvailable(_errOccured, _errMsg) ){
        return nullptr;
    }

    // init
    PArchiveHandler archiveHandler = std::make_shared<ArchiveHandler>( m_impl->commandServices );
    if( ! archiveHandler->init(_params) ){
        stringstream ss;
        ss << PRINT_HEADER << " archiver init ERROR [" << archiveHandler->getLastError() << "]";
        m_impl->lastError = ss.str();
        VS_LOG_ERROR << ss.str() << endl;
        if( _errMsg ){
            ( * _errMsg ) = ss.str();
        }
        if( _errOccured ){
            ( * _errOccured ) = true;
        }
        return nullptr;
    }

    // start
    archiveHandler->start();

    m_impl->archiveHandlersLock.lock();
    m_impl->archivingHandlersBySensorId.insert( {_params.sensorId, archiveHandler} );    
    m_impl->archiveHandlersLock.unlock();

    if( _errOccured ){
        ( * _errOccured ) = false;
    }
    return archiveHandler;
}

void VideoServerHandler::sendSignalStatusUpdated(){

    emit signalStatusUpdated();
}

void VideoServerHandler::sendSignalOnline( bool _online ){    
    emit signalOnline( _online );
}

void VideoServerHandler::sendSignalMessage( std::string _msg ){

    emit signalMessage( _msg.c_str() );
}

}
