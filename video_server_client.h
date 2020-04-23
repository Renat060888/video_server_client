#ifndef VIDEO_SERVER_CLIENT_H
#define VIDEO_SERVER_CLIENT_H

#include <string>
#include <memory>
#include <vector>

#include <boost/signals2.hpp>
#include <QString>
#include <QObject>

#include "common_types.h"
#include "video_server_handler.h"

namespace video_server_client{

class VideoServerClient : public QObject {
Q_OBJECT
    friend class PrivateImplementationVSC;
public:
    struct SInitSettings {
        SInitSettings()
            : signalNewServerDetected(nullptr)
        {}
        std::string gdmRootObjectName;
        boost::signals2::signal<void( TObjectId _serverId )> * signalNewServerDetected;
    };

    static VideoServerClient * getInstance();
    static void destroyInstance( VideoServerClient * & _inst );

    bool init( const SInitSettings & _settings );
    const std::string & getLastError();

    std::vector<PVideoServerHandler> getVideoServerHandlers();


signals:
    void signalClientMessage( QString _msg );
    void signalNewServerDetected( TObjectId _serverId );


private:
    void sendSignalClientMessage( std::string _msg );
    void sendSignalNewServerDetected( TObjectId _serverId );

    VideoServerClient();
    ~VideoServerClient();

    VideoServerClient( const VideoServerClient & inst ) = delete;
    const VideoServerClient & operator=( const VideoServerClient & inst ) = delete;

    class PrivateImplementationVSC * m_impl;
};

}

#endif // VIDEO_SERVER_CLIENT_H
