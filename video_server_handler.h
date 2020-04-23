#ifndef VIDEO_SERVER_HANDLER_H
#define VIDEO_SERVER_HANDLER_H

#include <memory>
#include <queue>
#include <unordered_map>
#include <future>

#include <QObject>

#include "from_ms_common/communication/network_interface.h"
#include "analyze_handler.h"
#include "archive_handler.h"
#include "commands/cmd_source_connect.h"

namespace video_server_client{

class VideoServerHandler : public QObject, public INetworkObserver
{
Q_OBJECT
    friend class PrivateImplementation;
    friend class AnalyzeHandler;
    friend class ArchiveHandler;
    friend class CommandExternalEvent;
    friend class CommandExternalState;
    friend class CommandAnalyzeStart;
    friend class CommandAnalyzeStop;
    friend class CommandAnalyzeStatus;
    friend class CommandArchiveStart;
    friend class CommandArchiveStop;
    friend class CommandArchivingStatus;
    friend class CommandPing;
public:
    struct SInitSettings {
        SInitSettings()
            : videoServerPongTimeoutSec(0)
        {}
        PNetworkClient networkClient;
        int64_t videoServerPongTimeoutSec;
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

    // retranslation
    bool sourceConnect( CommandConnectSource::SInitialParams _params );
    bool sourceDisconnect( CommandConnectSource::SInitialParams _params );

    // archive
    void destroyArchiving( PArchiveHandler _handler );
    std::unordered_multimap<TSensorId, PArchiveHandler> & getArchiveHandlers();
    std::unordered_map<TArchivingId, PArchiveHandler> & getArchiveHandlers2();
    PArchiveHandler launchArchiving( CommandArchiveStart::SInitialParams _params,
                                     bool * _errOccured = nullptr,
                                     std::string * _errMsg  = nullptr );


signals:
    void signalStatusUpdated();
    void signalOnline( bool _online );
    void signalMessage( QString _msg );


private:
    virtual void callbackNetworkRequest( PEnvironmentRequest _request ) override;

    void sendSignalStatusUpdated();
    void sendSignalOnline( bool _online );
    void sendSignalMessage( std::string _msg );


    bool init( SInitSettings _settings );
    SVideoServerStatus & getStatusRef();

    void setOwnIdByItSelf( const std::string _archivingId, uint64_t _corrId );
    void runPing();

    void runSystemClock();
    void checkConnectionState();
    void updateAnalyzeHandlers();
    void updateArchivingHandlers();
    void addDeferredSignalFuture( std::future<void> && _future );

    std::string getContextName( uint32_t _ctxId );

    struct VideoServerHandlerPrivate * m_impl;
};
using PVideoServerHandler = std::shared_ptr<VideoServerHandler>;

}
#endif // VIDEO_SERVER_HANDLER_H
