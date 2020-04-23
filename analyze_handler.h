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
    friend class VideoServerHandler;
public:

    AnalyzeHandler( common_types::SCommandServices & _commandServices );
    ~AnalyzeHandler();

    bool init( CommandAnalyzeStart::SInitialParams _params );
    const std::string & getLastError(){ return m_lastError; }
    const CommandAnalyzeStart::SInitialParams & getInitialParams(){ return m_initialParams; }

    bool start();
    bool stop( bool _destroy = false );
    SAnalyzeStatus getAnalyzeStatus();
    SAnalyzeLaunchParams getAnalyzeLaunchParams();
    std::vector<PConstAnalyticEvent> getEvents();


signals:
    void signalEventOccured();
    void signalStateChanged( video_server_client::EAnalyzeState _state );


private:
    void sendSignalEventOccured();
    void sendSignalStateChanged( const EAnalyzeState _state );

    void addEvent( PAnalyticEvent & event );

    void addStatus( const SAnalyzeStatus & _status, bool _afterDestroy );
    void statusChangedFromAsync( const SAnalyzeStatus & _status );
    void updateOnlyChangedValues( const SAnalyzeStatus & _statusIn, SAnalyzeStatus & _statusOut );

    // data
    std::vector<PConstAnalyticEvent> m_incomingEvents;
    std::string m_lastError;
    CommandAnalyzeStart::SInitialParams m_initialParams;
    SAnalyzeStatus m_status;
    EAnalyzeState m_stateToSignal;

    // service
    std::mutex m_mutexEventsLock;
    common_types::SCommandServices & m_commandServices;

    class AnalyzeHandlerPrivate * m_impl;
};
using PAnalyzeHandler = std::shared_ptr<AnalyzeHandler>;

}

#endif // PROCESSING_HANDLER_H
