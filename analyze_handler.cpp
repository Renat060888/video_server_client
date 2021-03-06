
#include <iostream>

#include "video_server_handler.h"
#include "commands/cmd_analyze_stop.h"
#include "from_ms_common/common/ms_common_utils.h"
#include "common_vars.h"
#include "common_utils.h"
#include "analyze_handler.h"

namespace video_server_client{

using namespace std;
using namespace common_types;

// ------------------------------------------------------------------------------
// private
// ------------------------------------------------------------------------------
class PrivateImplementationAnalyzeHandler {
public:
    PrivateImplementationAnalyzeHandler( SCommandServices & _commandServices )
        : m_commandServices(_commandServices)
        , interface(nullptr)
    {}


    // data
    std::vector<PConstAnalyticEvent> m_incomingEvents;
    std::string m_lastError;
    CommandAnalyzeStart::SInitialParams m_initialParams;
    SAnalyzeStatus m_status;
    EAnalyzeState m_stateToSignal;

    // service
    common_types::SCommandServices & m_commandServices;
    AnalyzeHandler * interface;
    std::mutex m_mutexEventsLock;
};


// ------------------------------------------------------------------------------
// public
// ------------------------------------------------------------------------------
AnalyzeHandler::AnalyzeHandler( SCommandServices & _commandServices )
    : m_impl(new PrivateImplementationAnalyzeHandler(_commandServices))
{
    m_impl->interface = this;
}

AnalyzeHandler::~AnalyzeHandler(){

    delete m_impl;
    m_impl = nullptr;
}

bool AnalyzeHandler::init( CommandAnalyzeStart::SInitialParams _params ){

    if( 0 == _params.sensorId ){
        m_impl->m_lastError = "sensor id is ZERO";
        return false;
    }
    if( 0 == _params.profileId ){
        m_impl->m_lastError = "profile id is ZERO";
        return false;
    }

    m_impl->m_initialParams = _params;

    cout << PRINT_HEADER
         << " created new analyze handler with sensor id [" << m_impl->m_initialParams.sensorId << "]"
         << endl;

    m_impl->m_status.sensorId = _params.sensorId;
    m_impl->m_status.processingId = _params.processingId;
    m_impl->m_status.processingName = _params.processingName;
    m_impl->m_status.profileId = _params.profileId;
    m_impl->m_status.analyzeState = _params.analyzeState;

    return true;
}

const std::string & AnalyzeHandler::getLastError(){
    return m_impl->m_lastError;
}

const CommandAnalyzeStart::SInitialParams & AnalyzeHandler::getInitialParams(){
    return m_impl->m_initialParams;
}

bool AnalyzeHandler::start(){

    // NOTE: выход из паузы определяется по текущему состоянию анализатора
    if( EAnalyzeState::READY == m_impl->m_status.analyzeState ){

        // TODO: move to VideoServerHandler as 'destroy' in STOP
        CommandAnalyzeStart::SInitialParams params;
        params.resume = true;
        params.sensorId = m_impl->m_status.sensorId;
        params.profileId = m_impl->m_status.profileId;
        params.processingId = m_impl->m_status.processingId;
        params.processingName = m_impl->m_status.processingName;

        PCommandAnalyzeStart cmd = std::make_shared<CommandAnalyzeStart>( & m_impl->m_commandServices );
        cmd->m_params = params;
        cmd->m_commandInitiator = this;
        const bool rt = cmd->execAsync();

        m_impl->m_commandServices.callbacks->addCommmand( cmd );

        return rt;
    }
    else{
        PCommandAnalyzeStart cmd = std::make_shared<CommandAnalyzeStart>( & m_impl->m_commandServices );
        cmd->m_params = m_impl->m_initialParams;
        cmd->m_commandInitiator = this;
        const bool rt = cmd->execAsync();

        m_impl->m_commandServices.callbacks->addCommmand( cmd );

        return rt;
    }
}

bool AnalyzeHandler::stop( bool _destroy ){

    CommandAnalyzeStop::SInitialParams params;
    params.sensorId = m_impl->m_status.sensorId;
    params.processingId = m_impl->m_status.processingId;
    params.destroy = _destroy;

    PCommandAnalyzeStop cmd = std::make_shared<CommandAnalyzeStop>( & m_impl->m_commandServices );
    cmd->m_params = params;
    const bool rt = cmd->execAsync();

    m_impl->m_commandServices.callbacks->addCommmand( cmd );

    return rt;
}

const SAnalyzeStatus & AnalyzeHandler::getAnalyzeStatus(){

    return m_impl->m_status;
}

SAnalyzeLaunchParams AnalyzeHandler::getAnalyzeLaunchParams(){

    SAnalyzeLaunchParams out;

    // TODO: temp
//    SAnalyzeStatus status = getAnalyzeStatus();
    out.processingId = m_impl->m_status.processingId;
    out.sensorId = m_impl->m_status.sensorId;

    return out;
}

std::vector<PConstAnalyticEvent> AnalyzeHandler::getEvents(){

    std::vector<PConstAnalyticEvent> out;

    m_impl->m_mutexEventsLock.lock();
    out.swap( m_impl->m_incomingEvents );
    m_impl->m_mutexEventsLock.unlock();

    return out;
}

void AnalyzeHandler::addStatus( const SAnalyzeStatus & _status, bool _afterDestroy ){

    updateOnlyChangedValues( _status, m_impl->m_status );

    // TODO: comment this snippet - what's going on here ?
    // signal about state change only on non-destroy event
    if( ! _afterDestroy ){
        auto lambda = [ this ]( const SAnalyzeStatus & _status ) {

            constexpr int delayMillisec = 3000;
            std::this_thread::sleep_for( std::chrono::milliseconds(delayMillisec) );
            sendSignalStateChanged( _status.analyzeState );
        };

        // create future for deferred signal
        std::future<void> deferredSignalFuture = std::async( std::launch::async, lambda, _status );
        m_impl->m_commandServices.callbacks->addFuture( std::move(deferredSignalFuture) );
    }
}

void AnalyzeHandler::updateOnlyChangedValues( const SAnalyzeStatus & _statusIn, SAnalyzeStatus & _statusOut ){

    if( _statusIn.sensorId != 0 ){
        _statusOut.sensorId = _statusIn.sensorId;
        m_impl->m_initialParams.sensorId = _statusIn.sensorId;
    }
    if( _statusIn.profileId != 0 ){
        _statusOut.profileId = _statusIn.profileId;
        m_impl->m_initialParams.profileId = _statusIn.profileId;
    }
    if( _statusIn.analyzeState != EAnalyzeState::UNDEFINED ){
        _statusOut.analyzeState = _statusIn.analyzeState;
        m_impl->m_initialParams.analyzeState = _statusIn.analyzeState;
    }
    if( ! _statusIn.processingId.empty() ){
        _statusOut.processingId = _statusIn.processingId;
        m_impl->m_initialParams.processingId = _statusIn.processingId;
    }
    if( ! _statusIn.processingName.empty() ){
        _statusOut.processingName = _statusIn.processingName;
        m_impl->m_initialParams.processingName = _statusIn.processingName;
    }
}

void AnalyzeHandler::addEvent( PAnalyticEvent & event ){

    m_impl->m_mutexEventsLock.lock();
    m_impl->m_incomingEvents.push_back( event );
    m_impl->m_mutexEventsLock.unlock();

    sendSignalEventOccured();
}


void AnalyzeHandler::sendSignalEventOccured(){
    emit signalEventOccured();
}

void AnalyzeHandler::sendSignalStateChanged( const EAnalyzeState _state ){

    // avoid duplicate
    if( m_impl->m_stateToSignal == _state ){
        return;
    }
    m_impl->m_stateToSignal = _state;

    // state hierarchy
    if( EAnalyzeState::ACTIVE == m_impl->m_stateToSignal && EAnalyzeState::PREPARING == _state ){
        cout << PRINT_HEADER
             << " catched state [PREPARING] after state [ACTIVE]. Signal aborted"
             << endl;
        return;
    }

    cout << PRINT_HEADER
         << " anayzer with sensor id [" << m_impl->m_initialParams.sensorId << "]"
         << " changed state to [" << common_utils::convertAnalyzeStateToStr(_state) << "]"
         << endl;    

    emit signalStateChanged( _state );
}

}





