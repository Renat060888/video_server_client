#ifndef PROCESSING_HANDLER_H
#define PROCESSING_HANDLER_H

#include <memory>
#include <mutex>

#include <QObject>

#include "commands/cmd_analyze_start.h"
#include "commands/cmd_analyze_status.h"
#include "common_types.h"

namespace video_server_client{

class AnalyzeHandler : public QObject
{
Q_OBJECT
    friend class PrivateImplementationVSH;
    friend class CommandAnalyzeStart;
public:
    AnalyzeHandler( common_types::SCommandServices & _commandServices );
    ~AnalyzeHandler();

    bool init( CommandAnalyzeStart::SInitialParams _params );
    const std::string & getLastError();
    const CommandAnalyzeStart::SInitialParams & getInitialParams();

    bool start();
    bool stop( bool _destroy = false );
    const SAnalyzeStatus & getAnalyzeStatus();
    SAnalyzeLaunchParams getAnalyzeLaunchParams();
    std::vector<PConstAnalyticEvent> getEvents();


signals:
    void signalEventOccured();
    void signalStateChanged( video_server_client::EAnalyzeState _state );


private:
    void sendSignalEventOccured();
    void sendSignalStateChanged( const EAnalyzeState _state );

    class PrivateImplementationAnalyzeHandler * m_impl;

    // access allowed only for friend entities
private:
    void addStatus( const SAnalyzeStatus & _status, bool _afterDestroy );
    void updateOnlyChangedValues( const SAnalyzeStatus & _statusIn, SAnalyzeStatus & _statusOut );
    void addEvent( PAnalyticEvent & event );
};
using PAnalyzeHandler = std::shared_ptr<AnalyzeHandler>;

}

#endif // PROCESSING_HANDLER_H
