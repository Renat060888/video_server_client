#ifndef VIDEO_SERVER_HANDLER_H
#define VIDEO_SERVER_HANDLER_H

#include <memory>
#include <queue>
#include <unordered_map>
#include <future>

#include <QObject>

#include "analyze_handler.h"
#include "archive_handler.h"
#include "commands/cmd_source_connect.h"

namespace video_server_client{

class VideoServerHandler : public QObject
{
Q_OBJECT
    friend class PrivateImplementationVSC;
    friend class PrivateImplementationVSH;
public:
    struct SInitSettings {
        SInitSettings()
            : objreprId(0)
            , role(EServerRole::UNDEFINED)
        {}
        PNetworkClient networkClient;
        TObjectId objreprId;
        EServerRole role;
    };

    VideoServerHandler();
    ~VideoServerHandler();

    const std::string & getLastError();
    PConstVideoServerStatus getStatus();

    // analyze
    void destroyAnalyze( PAnalyzeHandler _handler );
    std::unordered_multimap<TSensorId, PAnalyzeHandler> & getAnalyzeHandlers();
    std::unordered_map<TProcessingId, PAnalyzeHandler> & getAnalyzeHandlers2();
    PAnalyzeHandler launchAnalyze( CommandAnalyzeStart::SInitialParams _params,
                                   bool * _errOccured = nullptr,
                                   std::string * _errMsg  = nullptr );

    // archive
    void destroyArchiving( PArchiveHandler _handler );
    std::unordered_multimap<TSensorId, PArchiveHandler> & getArchiveHandlers();
    std::unordered_map<TArchivingId, PArchiveHandler> & getArchiveHandlers2();
    PArchiveHandler launchArchiving( CommandArchiveStart::SInitialParams _params,
                                     bool * _errOccured = nullptr,
                                     std::string * _errMsg  = nullptr );

    // retranslation
    bool sourceConnect( CommandConnectSource::SInitialParams _params );
    bool sourceDisconnect( CommandConnectSource::SInitialParams _params );


signals:
    void signalStatusUpdated();
    void signalOnline( bool _online );
    void signalMessage( QString _msg );


private:   
    void sendSignalStatusUpdated();
    void sendSignalOnline( bool _online );
    void sendSignalMessage( std::string _msg );

    class PrivateImplementationVSH * m_impl;

    // access allowed only for private entities
private:
    bool init( const SInitSettings & _settings );
    SVideoServerStatus & getStatusRef();
    void runSystemClock();
};
using PVideoServerHandler = std::shared_ptr<VideoServerHandler>;

}
#endif // VIDEO_SERVER_HANDLER_H
