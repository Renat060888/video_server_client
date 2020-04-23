#ifndef ARCHIVE_HANDLER_H
#define ARCHIVE_HANDLER_H

#include <QObject>

#include "commands/cmd_archive_start.h"
#include "commands/cmd_archiving_status.h"
#include "common_types.h"

namespace video_server_client{

class ArchiveHandler : public QObject
{
Q_OBJECT
    friend class VideoServerHandler;
public:
    ArchiveHandler( SCommandServices & _commandServices );

    bool init( CommandArchiveStart::SInitialParams _params );
    const CommandArchiveStart::SInitialParams & getInitialParams(){ return m_initialParams; }
    const std::string & getLastError(){ return m_lastError; }

    bool start();
    bool stop( bool _destroy = false );

    SArchiveStatus getArchiveStatus();

signals:
    void signalStateChanged( video_server_client::EArchiveState _state );

private:
    void sendSignalStateChanged( const EArchiveState _state );

    void statusChangedFromAsync( const SArchiveStatus & _status );
    void addStatus( const SArchiveStatus & _status, bool _afterDestroy );

    void updateOnlyChangedValues( const SArchiveStatus & _statusIn, SArchiveStatus & _statusOut );

    bool startAsync();
    bool stopAsync( bool _destroy = false );

    // data
    CommandArchiveStart::SInitialParams m_initialParams;
    SArchiveStatus m_status;
    std::string m_lastError;
    EArchiveState m_stateToSignal;

    // service
    SCommandServices & m_commandServices;

    class ArchiveHandlerPrivate * m_impl;
};
using PArchiveHandler = std::shared_ptr<ArchiveHandler>;

}

#endif // ARCHIVE_HANDLER_H
