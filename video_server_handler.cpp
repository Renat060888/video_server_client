
#include <objrepr/reprServer.h>

#include "from_ms_common/common/ms_common_utils.h"
#include "from_ms_common/communication/network_interface.h"
#include "from_ms_common/system/logger.h"
#include "from_ms_common/system/object_pool.h"
#include "commands/cmd_ping.h"
#include "common_vars.h"
#include "video_server_handler.h"

static constexpr int64_t PING_INTERVAL_MILLISEC = 1000;
static constexpr int64_t SERVER_TIMEOUT_MILLISEC = 30000;

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
    {}

    virtual void pongCatched() override {
        lastPongMillisec = common_utils::getCurrentTimeMillisec();
    }

    virtual void archiverIsReady( const TArchivingId & _archId, const TLaunchCorrelationId & _corrId ) override {

        auto iter = archiversAtLaunchPhase.find( _corrId );
        if( iter != archiversAtLaunchPhase.end() ){
            PArchiveHandler archiverHandler = iter->second;
            archiverHandler->setArchivingId( _archId );

            archiversAtLaunchPhase.erase( iter );
        }
        else{
            // TODO: do
        }
    }

    virtual void analyzerIsReady( const TProcessingId & _procId, const TLaunchCorrelationId & _corrId ) override {

        auto iter = analyzersAtLaunchPhase.find( _corrId );
        if( iter != analyzersAtLaunchPhase.end() ){
            PAnalyzeHandler analyzerHandler = iter->second;
            analyzerHandler->setProcessingId( _procId );

            analyzersAtLaunchPhase.erase( iter );
        }
        else{
            // TODO: do
        }
    }

    virtual void updateServerState( const SServerState & _state ) override {
        // TODO: assign by one variable
        status->objreprId = _state.objreprId;
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

        for( const SArchiveStatus & state : _status ){
            archiveHandlersLock.lock();
            auto iter = archivingHandlersByArchivingId.find( state.archivingId );
            if( iter != archivingHandlersByArchivingId.end() ){
                PArchiveHandler archiverHandler = iter->second;

                archiverHandler->addStatus( state, false );
            }
            archiveHandlersLock.unlock();
        }
    }

    virtual void updateAnalyzeStatus( const std::vector<SAnalyzeStatus> & _status ) override {

        for( const SAnalyzeStatus & state : _status ){
            analyzeHandlersLock.lock();
            auto iter = analyzeHandlersByProcessingId.find( state.processingId );
            if( iter != analyzeHandlersByProcessingId.end() ){
                PAnalyzeHandler analyzeHandler = iter->second;

                analyzeHandler->addStatus( state, false );
            }
            analyzeHandlersLock.unlock();
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

    virtual void callbackNetworkRequest( PEnvironmentRequest _request ) override {

        VS_LOG_INFO << PRINT_HEADER
                    << " this method highly likely is not necessary"
                    << " , but msg [" << _request->m_incomingMessage << "]"
                    << endl;
    }

    void runSystemClock(){

        networkProvider->runNetworkCallbacks();
        commandsProcessing();
        remoteServicesMonitoring();
        ping();
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
                online = false;
                interface->sendSignalOnline( false );
            }
        }
        else{
            if( (common_utils::getCurrentTimeMillisec() - lastPongMillisec) < SERVER_TIMEOUT_MILLISEC ){
                VS_LOG_INFO << PRINT_HEADER << " VideoServer ONLINE at " << common_utils::getCurrentDateTimeStr() << endl;
                online = true;
                interface->sendSignalOnline( true );
            }
        }
    }

    inline void ping(){

        if( (common_utils::getCurrentTimeMillisec() - lastPingAtMillisec) > PING_INTERVAL_MILLISEC ){
            if( commandPing->isReady() ){
                commandPing->execAsync();
            }

            lastPingAtMillisec = common_utils::getCurrentTimeMillisec();
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
    std::map<TLaunchCorrelationId, PArchiveHandler> archiversAtLaunchPhase;
    std::map<TLaunchCorrelationId, PAnalyzeHandler> analyzersAtLaunchPhase;

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

    if( ! m_impl->isServerAvailable(_errOccured, _errMsg) ){
        return nullptr;
    }

    // TODO: do



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

    // launch correlation
    const TLaunchCorrelationId corrId = common_utils::generateUniqueId();
    m_impl->archiversAtLaunchPhase.insert( {corrId, archiveHandler} );

    // start
    archiveHandler->start();

    m_impl->archiveHandlersLock.lock();
    m_impl->archivingHandlersBySensorId.insert( {_params.sensorId, archiveHandler} );
    m_impl->archivingHandlersByArchivingId.insert( {archiveHandler->getArchiveStatus().archivingId, archiveHandler} );
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
