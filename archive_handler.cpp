
#include <thread>

#include "from_ms_common/common/ms_common_utils.h"
#include "common_types_private.h"
#include "common_vars.h"
#include "video_server_handler.h"
#include "archive_handler.h"
#include "commands/cmd_archive_start.h"
#include "commands/cmd_archive_stop.h"
#include "commands/cmd_archiving_status.h"

namespace video_server_client{

using namespace std;

// ------------------------------------------------------------------------------
// private
// ------------------------------------------------------------------------------
class ArchiveHandlerPrivate {
public:




};


// ------------------------------------------------------------------------------
// public
// ------------------------------------------------------------------------------
ArchiveHandler::ArchiveHandler( SCommandServices & _commandServices )
    : m_commandServices(_commandServices)
{

}

bool ArchiveHandler::init( CommandArchiveStart::SInitialParams _params ){

    if( 0 == _params.sensorId ){
        m_lastError = "sensor id is ZERO";
        return false;
    }

    if( _params.archivingName.empty() ){
        m_lastError = "archiving name is EMPTY";
        return false;
    }

    m_initialParams = _params;

    cout << common_vars::PRINT_HEADER
         << " created new archive handler with sensor id [" << m_initialParams.sensorId << "]"
         << endl;

    m_status.sensorId = _params.sensorId;
    m_status.archivingId = _params.archivingId;
    m_status.archivingName = _params.archivingName;
    m_status.archiveState = _params.archiveState;

    return true;
}

bool ArchiveHandler::startAsync(){

    m_commandServices.muAsyncOperations.lock();
    const bool empty = m_commandServices.asyncStartOperations.empty();
    m_commandServices.muAsyncOperations.unlock();
    if( ! empty ){
        cerr << common_vars::PRINT_HEADER << " WARNING, previous 'start' command is not yet completed" << endl;
        return false;
    }

    std::future<bool> startInFuture = std::async( std::launch::async, [this](){

        PCommandArchiveStart cmd = std::make_shared<CommandArchiveStart>( & m_commandServices );
        if( ! cmd->init( m_initialParams ) ){
            m_lastError = cmd->getLastError();
            return false;
        }

        const bool rt = cmd->exec();

        m_status.archivingId = cmd->m_archivingId;
        m_status.archiveState = cmd->m_archiveState;

        // NOTE: при первом запуске "archivingId" будет пустой, что значит создать стартовый архив.
        // При повторных же с существующим ID будет создаваться архив с таким же ID,
        // пока архиватор не будет уничтожен через "destroyArchiving" клиентским кодом
        m_initialParams.archivingId = m_status.archivingId;

        // set own id by it self
        m_commandServices.handler->setOwnIdByItSelf( m_status.archivingId, m_initialParams.correlationId );

        return rt;
    } );

    // push into futureQueue
    m_commandServices.muAsyncOperations.lock();
    m_commandServices.asyncStartOperations.push_back( std::move(startInFuture) );
    m_commandServices.muAsyncOperations.unlock();

    return true;
}

bool ArchiveHandler::stopAsync( bool _destroy ){

    m_commandServices.muAsyncOperations.lock();
    const bool empty = m_commandServices.asyncStopOperations.empty();
    m_commandServices.muAsyncOperations.unlock();
    if( ! empty ){
        cerr << common_vars::PRINT_HEADER << " WARNING, previous 'stop' command is not yet completed" << endl;
        return false;
    }

    std::future<bool> stopInFuture = std::async( std::launch::async, [this, _destroy](){

        PCommandArchiveStop cmd = std::make_shared<CommandArchiveStop>( & m_commandServices );
        CommandArchiveStop::SInitialParams params;
        params.archivingId = m_status.archivingId;
        params.destroy = _destroy;
        if( ! cmd->init( params ) ){
            m_lastError = cmd->getLastError();
            return false;
        }

        return cmd->exec();
    } );

    // push into futureQueue
    m_commandServices.muAsyncOperations.lock();
    m_commandServices.asyncStopOperations.push_back( std::move(stopInFuture) );
    m_commandServices.muAsyncOperations.unlock();

    return true;
}

bool ArchiveHandler::start(){

    if( m_initialParams.asyncOperations ){
        return startAsync();
    }

    PCommandArchiveStart cmd = std::make_shared<CommandArchiveStart>( & m_commandServices );
    if( ! cmd->init( m_initialParams ) ){
        m_lastError = cmd->getLastError();
        return false;
    }

    const bool rt = cmd->exec();

    m_status.archivingId = cmd->m_archivingId;
    m_status.archiveState = cmd->m_archiveState;

    // NOTE: при первом запуске "archivingId" будет пустой, что значит создать стартовый архив.
    // При повторных же с существующим ID будет создаваться архив с таким же ID,
    // пока архиватор не будет уничтожен через "destroyArchiving" клиентским кодом
    m_initialParams.archivingId = m_status.archivingId;

    return rt;
}

bool ArchiveHandler::stop( bool _destroy ){

    if( m_initialParams.asyncOperations ){
        return stopAsync( _destroy );
    }

    PCommandArchiveStop cmd = std::make_shared<CommandArchiveStop>( & m_commandServices );
    CommandArchiveStop::SInitialParams params;
    params.archivingId = m_status.archivingId;
    params.destroy = _destroy;
    if( ! cmd->init( params ) ){
        m_lastError = cmd->getLastError();
        return false;
    }

    return cmd->exec();
}

void ArchiveHandler::sendSignalStateChanged( const EArchiveState _state ){

    // avoid duplicate
    if( m_stateToSignal == _state ){
        return;
    }
    m_stateToSignal = _state;

    // state hierarchy
    if( EArchiveState::ACTIVE == m_stateToSignal && EArchiveState::PREPARING == _state ){
        cout << common_vars::PRINT_HEADER
             << " catched state [PREPARING] after state [ACTIVE]. Signal aborted"
             << endl;
        return;
    }

    cout << common_vars::PRINT_HEADER
         << " archiver with sensor id [" << m_initialParams.sensorId << "]"
         << " changed state to [" << common_utils::convertArchivingStateToStr(_state) << "]"
         << endl;

    emit signalStateChanged(_state);
}

void ArchiveHandler::addStatus( const SArchiveStatus & _status, bool _afterDestroy ){

    updateOnlyChangedValues( _status, m_status );

    // signal about state change only on non-destroy event
    if( ! _afterDestroy ){
        auto lambda = [ this ]( const SArchiveStatus & _status ) {

            constexpr int delayMillisec = 3000;
            std::this_thread::sleep_for( std::chrono::milliseconds(delayMillisec) );
            sendSignalStateChanged( _status.archiveState );
        };

        // create future for deferred signal
        std::future<void> deferredSignalFuture = std::async( std::launch::async, lambda, _status );
        m_commandServices.handler->addDeferredSignalFuture( std::move(deferredSignalFuture) );
    }
}

void ArchiveHandler::statusChangedFromAsync( const SArchiveStatus & _status ){

    updateOnlyChangedValues( _status, m_status );
    sendSignalStateChanged( _status.archiveState );
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

SArchiveStatus ArchiveHandler::getArchiveStatus(){
    return m_status;
}

}
