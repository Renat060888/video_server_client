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
    friend class PrivateImplementationVSH;
    friend class CommandArchiveStart;
public:
    ArchiveHandler( common_types::SCommandServices & _commandServices );
    ~ArchiveHandler();

    bool init( CommandArchiveStart::SInitialParams _params );
    const CommandArchiveStart::SInitialParams & getInitialParams();
    const std::string & getLastError();

    bool start();
    bool stop( bool _destroy = false );

    const SArchiveStatus & getArchiveStatus();


signals:
    void signalStateChanged( video_server_client::EArchiveState _state );


private:
    void sendSignalStateChanged( const EArchiveState _state );

    class PrivateImplementationArchiveHandler * m_impl;

    // access allowed only for friend entities
private:
    void addStatus( const SArchiveStatus & _status, bool _afterDestroy );
    void updateOnlyChangedValues( const SArchiveStatus & _statusIn, SArchiveStatus & _statusOut );
};
using PArchiveHandler = std::shared_ptr<ArchiveHandler>;

}

#endif // ARCHIVE_HANDLER_H
