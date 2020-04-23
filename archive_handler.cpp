
#include <thread>

#include "from_ms_common/system/logger.h"
#include "from_ms_common/common/ms_common_utils.h"
#include "common_types_private.h"
#include "common_vars.h"
#include "common_utils.h"
#include "video_server_handler.h"
#include "commands/cmd_archive_start.h"
#include "commands/cmd_archive_stop.h"
#include "commands/cmd_archiving_status.h"
#include "archive_handler.h"

namespace video_server_client{

using namespace std;
using namespace common_types;

// ------------------------------------------------------------------------------
// private
// ------------------------------------------------------------------------------
class PrivateImplementationArchiveHandler {
public:
    PrivateImplementationArchiveHandler( SCommandServices & _commandServices )
        : m_commandServices(_commandServices)
        , interface(nullptr)
    {}




    // data
    CommandArchiveStart::SInitialParams m_initialParams;
    SArchiveStatus m_status;
    std::string m_lastError;
    EArchiveState m_stateToSignal;

    // service
    ArchiveHandler * interface;
    common_types::SCommandServices & m_commandServices;
};


// ------------------------------------------------------------------------------
// public
// ------------------------------------------------------------------------------
ArchiveHandler::ArchiveHandler( SCommandServices & _commandServices )
    : m_impl(new PrivateImplementationArchiveHandler(_commandServices))
{
    m_impl->interface = this;
}

ArchiveHandler::~ArchiveHandler(){

    delete m_impl;
    m_impl = nullptr;
}

bool ArchiveHandler::init( CommandArchiveStart::SInitialParams _params ){

    // check stage
    if( 0 == _params.sensorId ){
        m_impl->m_lastError = "sensor id is ZERO";
        return false;
    }
    if( _params.archivingName.empty() ){
        m_impl->m_lastError = "archiving name is EMPTY";
        return false;
    }

    // for 'start' command
    m_impl->m_initialParams = _params;

    //
    m_impl->m_status.sensorId = _params.sensorId;
    m_impl->m_status.archivingId = _params.archivingId;
    m_impl->m_status.archivingName = _params.archivingName;
    m_impl->m_status.archiveState = _params.archiveState;

    VS_LOG_INFO << PRINT_HEADER
            << " created new archive handler with sensor id [" << m_impl->m_initialParams.sensorId << "]"
            << endl;
    return true;
}

const CommandArchiveStart::SInitialParams & ArchiveHandler::getInitialParams(){

    return m_impl->m_initialParams;
}

const std::string & ArchiveHandler::getLastError(){

    return m_impl->m_lastError;
}

bool ArchiveHandler::start(){

    PCommandArchiveStart cmd = std::make_shared<CommandArchiveStart>( & m_impl->m_commandServices );
    cmd->m_params = m_impl->m_initialParams;
    const bool rt = cmd->exec();

    m_impl->m_status.archivingId = cmd->m_archivingId;
    m_impl->m_status.archiveState = cmd->m_archiveState;

    // NOTE: при первом запуске "archivingId" будет пустой, что значит создать стартовый архив.
    // При повторных же с существующим ID будет создаваться архив с таким же ID,
    // пока архиватор не будет уничтожен через "destroyArchiving" клиентским кодом
    m_impl->m_initialParams.archivingId = m_impl->m_status.archivingId;

    return rt;
}

bool ArchiveHandler::stop( bool _destroy ){

    CommandArchiveStop::SInitialParams params;
    params.archivingId = m_impl->m_status.archivingId;
    params.destroy = _destroy;

    PCommandArchiveStop cmd = std::make_shared<CommandArchiveStop>( & m_impl->m_commandServices );
    cmd->m_params = params;
    return cmd->exec();
}

void ArchiveHandler::sendSignalStateChanged( const EArchiveState _state ){

    // avoid duplicate
    if( m_impl->m_stateToSignal == _state ){
        return;
    }
    m_impl->m_stateToSignal = _state;

    // state hierarchy
    if( EArchiveState::ACTIVE == m_impl->m_stateToSignal && EArchiveState::PREPARING == _state ){
        cout << PRINT_HEADER
             << " catched state [PREPARING] after state [ACTIVE]. Signal aborted"
             << endl;
        return;
    }

    cout << PRINT_HEADER
         << " archiver with sensor id [" << m_impl->m_initialParams.sensorId << "]"
         << " changed state to [" << common_utils::convertArchivingStateToStr(_state) << "]"
         << endl;

    emit signalStateChanged(_state);
}

void ArchiveHandler::setArchivingId( const TArchivingId & _id ){

    m_impl->m_status.archivingId = _id;
}

void ArchiveHandler::addStatus( const SArchiveStatus & _status, bool _afterDestroy ){

    updateOnlyChangedValues( _status, m_impl->m_status );

    // TODO: comment this snippet - what's going on here ?
    // signal about state change only on non-destroy event
    if( ! _afterDestroy ){
        auto lambda = [ this ]( const SArchiveStatus & _status ) {

            constexpr int delayMillisec = 3000;
            std::this_thread::sleep_for( std::chrono::milliseconds(delayMillisec) );
            sendSignalStateChanged( _status.archiveState );
        };

        // create future for deferred signal
        std::future<void> deferredSignalFuture = std::async( std::launch::async, lambda, _status );
//        m_impl->m_commandServices.handler->addDeferredSignalFuture( std::move(deferredSignalFuture) );
    }
}

void ArchiveHandler::updateOnlyChangedValues( const SArchiveStatus & _statusIn, SArchiveStatus & _statusOut ){

    if( _statusIn.sensorId != 0 ){
        _statusOut.sensorId = _statusIn.sensorId;
    }
    if( _statusIn.archiveState != EArchiveState::UNDEFINED ){
        _statusOut.archiveState = _statusIn.archiveState;
    }
    if( ! _statusIn.archivingId.empty() ){
        _statusOut.archivingId = _statusIn.archivingId;
    }
    if( ! _statusIn.archivingName.empty() ){
        _statusOut.archivingName = _statusIn.archivingName;
    }
}

const SArchiveStatus & ArchiveHandler::getArchiveStatus(){

    return m_impl->m_status;
}

}
