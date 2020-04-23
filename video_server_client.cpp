
#include "from_ms_common/communication/amqp_client_c.h"
#include "from_ms_common/communication/amqp_controller.h"
#include "from_ms_common/system/logger.h"
#include "from_ms_common/common/ms_common_utils.h"
#include "common_vars.h"
#include "video_server_client.h"

using namespace std;

namespace video_server_client{

static VideoServerClient * g_instance = nullptr;
static int32_t g_instanceRefCounter = 0;

static constexpr int64_t SERVER_TIMEOUT_MILLISEC = 60000;

// --------------------------------------------------------------------
// private
// --------------------------------------------------------------------
class PrivateImplementation {
public:
    struct SVideoServerConnection {
        SVideoServerConnection()
            : videoServerPort(0)
            , online(false)
            , objreprId(0)
            , role(EServerRole::UNDEFINED)
        {}
        std::string videoServerHost;
        int32_t videoServerPort;
        bool online;
        TObjectId objreprId;
        EServerRole role;
    };

    struct SAmqpSettings {
        SAmqpSettings()
            : amqpBrokerPort(0)
        {}
        std::string amqpBrokerHost;
        std::string amqpBrokerVirtualHost;
        int amqpBrokerPort;
        std::string amqpLogin;
        std::string amqpPass;
    };

    PrivateImplementation()
        : interface(nullptr)
        , threadClientMaintenance(nullptr)
        , shutdownCalled(false)
        , inited(false)
    {

    }

    ~PrivateImplementation(){


    }

    std::vector<PrivateImplementation::SVideoServerConnection> findVideoServers(){
        const std::string VIDEO_SERVER_INSTANCE_AN_CLASSINFO_NAME = "Сервер обработки видео";
        const std::string VIDEO_SERVER_INSTANCE_AR_CLASSINFO_NAME = "Сервер хранения видео";
        const std::string VIDEO_SERVER_INSTANCE_ATTR_ID = "Идентификатор";
        const std::string VIDEO_SERVER_INSTANCE_ATTR_ADDRESS = "Адрес";

        // find root object
        objrepr::SpatialObjectManager * objManager = objrepr::RepresentationServer::instance()->objectManager();
        objrepr::GlobalDataManager * globalManager = objrepr::RepresentationServer::instance()->globalManager();

        std::vector<PrivateImplementation::SVideoServerConnection> out;

        // find or create new root container
        vector<objrepr::SpatialObjectPtr> videoServersContainerInstances;
        objrepr::ClassinfoPtr videoServerAnalyzeInstanceClassinfo;
        objrepr::ClassinfoPtr videoServerArchiveInstanceClassinfo;
        bool videoServerContainerClassinfoFound = false;

        vector<objrepr::ClassinfoPtr> classes = globalManager->classinfoListByCat( "container" );
        for( objrepr::ClassinfoPtr & currentClassinfo : classes ){

            if( std::string(currentClassinfo->name()) == settings.gdmRootObjectName ){

                videoServerContainerClassinfoFound = true;
                vector<objrepr::SpatialObjectPtr> rootClassinfoObjects =  objManager->getObjectsByClassinfo( currentClassinfo->id() );
                // use existing container
                if( ! rootClassinfoObjects.empty() ){
                    if( rootClassinfoObjects.size() > 1 ){
                        VS_LOG_ERROR << PRINT_HEADER
                             << "MORE THAN ONE objrepr gdm-video-objects-container instances is detected"
                             << endl;
                        return out;
                    }
                    else{
                        videoServersContainerInstances.insert( videoServersContainerInstances.end(),
                                                               rootClassinfoObjects.begin(),
                                                               rootClassinfoObjects.end() );
                    }
                }
            }

            if( std::string(currentClassinfo->name()) == VIDEO_SERVER_INSTANCE_AN_CLASSINFO_NAME ){
                videoServerAnalyzeInstanceClassinfo = currentClassinfo;
            }

            if( std::string(currentClassinfo->name()) == VIDEO_SERVER_INSTANCE_AR_CLASSINFO_NAME ){
                videoServerArchiveInstanceClassinfo = currentClassinfo;
            }
        }

        // VS-container classinfo
        if( ! videoServerContainerClassinfoFound ){
            VS_LOG_ERROR << PRINT_HEADER << "objrepr gdm-video-objects-container classinfo not found: " << settings.gdmRootObjectName
                 << endl;
            return out;
        }

        // VS-container instance
        if( videoServersContainerInstances.empty() ){
            VS_LOG_ERROR << PRINT_HEADER << "objrepr gdm-video-objects-container instance not found, classinfo: " << settings.gdmRootObjectName
                 << endl;
            return out;
        }

        // VS-object classinfo
        if( ! videoServerAnalyzeInstanceClassinfo || ! videoServerArchiveInstanceClassinfo ){
            VS_LOG_ERROR << PRINT_HEADER << "objrepr gdm-video-server classinfo not found: " << VIDEO_SERVER_INSTANCE_AN_CLASSINFO_NAME
                 << endl;
            return out;
        }

        // accumulate VS-object instances from containers
        std::vector<objrepr::SpatialObjectPtr> mayBeVideoServers;
        for( const objrepr::SpatialObjectPtr & rootContainer : videoServersContainerInstances ){
            const std::vector<objrepr::SpatialObjectPtr> mayBeVS = objManager->getObjectsByParent( rootContainer->id() );
            mayBeVideoServers.insert( mayBeVideoServers.end(), mayBeVS.begin(), mayBeVS.end() );
        }

        // VS-object instances
        for( objrepr::SpatialObjectPtr object : mayBeVideoServers ){
            if( std::string(object->classinfo()->name()) == videoServerAnalyzeInstanceClassinfo->name() ||
                std::string(object->classinfo()->name()) == videoServerArchiveInstanceClassinfo->name() ){
                objrepr::SpatialObjectPtr videoServer = object;

                objrepr::StringAttributePtr idAttr = boost::dynamic_pointer_cast<objrepr::StringAttribute>( videoServer->attrMap()->getAttr(VIDEO_SERVER_INSTANCE_ATTR_ID.c_str()) );
                if( ! idAttr ){
                    VS_LOG_ERROR << PRINT_HEADER << "objrepr gdm-video-server instance attr not found: " << VIDEO_SERVER_INSTANCE_ATTR_ID
                         << endl;
                    return out;
                }

                objrepr::StringAttributePtr addressAttr = boost::dynamic_pointer_cast<objrepr::StringAttribute>( videoServer->attrMap()->getAttr(VIDEO_SERVER_INSTANCE_ATTR_ADDRESS.c_str()) );
                if( ! addressAttr ){
                    VS_LOG_ERROR << PRINT_HEADER << "objrepr gdm-video-server instance attr not found: " << VIDEO_SERVER_INSTANCE_ATTR_ADDRESS
                         << endl;
                    return out;
                }

                const std::string address = addressAttr->valueAsString();

                PrivateImplementation::SVideoServerConnection connection;
                connection.videoServerHost = address.substr( 0, address.find_first_of(":") );
                connection.videoServerPort = stoi( address.substr( address.find_first_of(":") + 1, address.size() - address.find_first_of(":") ) );
                connection.objreprId = videoServer->id();

                if( std::string(object->classinfo()->name()) == videoServerAnalyzeInstanceClassinfo->name() ){
                    connection.role = EServerRole::ANALYZE;
                }
                else{
                    connection.role = EServerRole::ARCHIVING;
                }

                // get flag online
                objrepr::PhysModelPtr physicModel = videoServer->classinfo()->phm();
                objrepr::DynBooleanAttributePtr onlineAttr = boost::dynamic_pointer_cast<objrepr::DynBooleanAttribute>( physicModel->instanceAttrMap()->getAttr(common_vars::VIDEO_SERVER_DYNATTR_ONLINE.c_str()) );
                connection.online = onlineAttr->value();

                out.push_back( connection );
            }
        }

        return out;
    }

    std::vector<PVideoServerHandler> connectToVideoServers( const std::vector<SVideoServerConnection> & _connections ){
        std::vector<PVideoServerHandler> out;
        for( const SVideoServerConnection & conn : _connections ){
            // network for connection w/server
            SAmqpSettings settings;
            settings.amqpBrokerHost = "";
            settings.amqpBrokerPort = 0;
            settings.amqpBrokerVirtualHost = "";
            settings.amqpLogin = "";
            settings.amqpPass = "";
            PNetworkClient networkTransport = createTransport( settings );
            PNetworkClient networkConnection = listenServer( networkTransport, conn.videoServerHost, conn.videoServerPort, conn.objreprId );
            if( ! networkConnection ){
                VS_LOG_WARN << PRINT_HEADER << " couldn't create video-server-handler for"
                     << " [" << conn.videoServerHost << ":" << conn.videoServerPort << "]"
                     << " because network failed"
                     << endl;
                continue;
            }

            // interaction w/server
            VideoServerHandler::SInitSettings handlerSettings;
            handlerSettings.networkClient = networkConnection;
            handlerSettings.videoServerPongTimeoutSec = SERVER_TIMEOUT_MILLISEC;
            handlerSettings.objreprId = conn.objreprId;
            handlerSettings.role = conn.role;
            PVideoServerHandler videoServerHandler = std::make_shared<VideoServerHandler>();
            if( ! videoServerHandler->init(handlerSettings) ){
                continue;
            }

            out.push_back( videoServerHandler );
        }

        return out;
    }

    PNetworkClient listenServer( const PNetworkClient & _networkClient, const std::string & _host, int _port, TObjectId _serverId ){
        const string linkId = std::to_string( _serverId );

        // client -> server
        SAmqpRouteParameters routes;
        routes.predatorExchangePointName = "video_dx_clients";
        routes.predatorQueueName = "video_q_client_mailbox_" + linkId;
        routes.predatorRoutingKeyName = "video_rk_to_client_" + linkId;
        routes.targetExchangePointName = "video_dx_servers";
        routes.targetQueueName = "video_q_server_mailbox_" + linkId;
        routes.targetRoutingKeyName = "video_rk_to_server_" + linkId;

        AmqpController::SInitSettings settings2;
        settings2.client = _networkClient;
        settings2.route = routes;

        PAmqpController controller = std::make_shared<AmqpController>( INetworkEntity::INVALID_CONN_ID );
        const bool rt = controller->init( settings2 );
        if( ! rt ){
            state->lastError = controller->getState().lastError;
            return nullptr;
        }

        return controller;
    }

    PNetworkClient createTransport( const SAmqpSettings & _settings ){
        PAmqpClient client = std::make_shared<AmqpClient>( INetworkEntity::INVALID_CONN_ID );

        AmqpClient::SInitSettings clientSettings;
        clientSettings.serverHost = _settings.amqpBrokerHost;
        clientSettings.amqpVirtualHost = _settings.amqpBrokerVirtualHost;
        clientSettings.port = _settings.amqpBrokerPort;
        clientSettings.login = _settings.amqpLogin;
        clientSettings.pass = _settings.amqpPass;
        clientSettings.deliveredMessageExpirationSec = 60;

        if( ! client->init(clientSettings) ){
            lastError = client->getState().m_lastError;
            return nullptr;
        }

        return client;
    }

    void slotContextLoaded(){
        const vector<PrivateImplementation::SVideoServerConnection> connections = findVideoServers();
        const std::vector<PVideoServerHandler> handlers = connectToVideoServers( connections );

        muVideoServerHandlers.lock();
        videoServerHandlers = handlers;
        muVideoServerHandlers.unlock();
    }

    void slotContextUnloaded(){
        muVideoServerHandlers.lock();
        videoServerHandlers.clear();
        muVideoServerHandlers.unlock();
    }

    void threadMaintenance(){
        while( ! shutdownCalled ){
            muVideoServerHandlers.lock();
            for( PVideoServerHandler & handler : videoServerHandlers ){
                handler->runSystemClock();
            }
            muVideoServerHandlers.unlock();

            this_thread::sleep_for( chrono::milliseconds(10) );
        }
    }

    // data
    bool inited;
    bool shutdownCalled;
    std::vector<PVideoServerHandler> videoServerHandlers;
    std::string lastError;

    // service
    VideoServerClient * interface;
    std::mutex muVideoServerHandlers;
    std::thread * threadClientMaintenance;
};


// --------------------------------------------------------------------
// public
// --------------------------------------------------------------------
VideoServerClient::VideoServerClient()
    : m_impl(new PrivateImplementation())
{
     m_impl->interface = this;
}

VideoServerClient::~VideoServerClient(){
    m_impl->shutdownCalled = true;
    common_utils::threadShutdown( m_impl->threadClientMaintenance );

    delete m_impl;
    m_impl = nullptr;
}

VideoServerClient * VideoServerClient::getInstance(){
    if( g_instance ){
        g_instanceRefCounter++;
        return g_instance;
    }
    else{
        g_instance = new VideoServerClient();
        g_instanceRefCounter++;

        VS_LOG_INFO << PRINT_HEADER << " instance created" << endl;

        return g_instance;
    }
}

bool VideoServerClient::destroyInstance( VideoServerClient * & _instance ){
    if( ! _instance ){
        return;
    }

    assert( g_instanceRefCounter > 0 );

    g_instanceRefCounter--;
    _instance = nullptr;

    if( 0 == g_instanceRefCounter ){
        delete g_instance;
        g_instance = nullptr;

        VS_LOG_INFO << PRINT_HEADER << " instance destroyed" << endl;
    }
}

bool VideoServerClient::init( const SInitSettings & _settings ){
    // check stage
    if( m_impl->inited ){
        VS_LOG_WARN << PRINT_HEADER << " already inited" << endl;
        return true;
    }

    if( _settings.gdmRootObjectName.empty() ){
        VS_LOG_ERROR << PRINT_HEADER
                     << " gdm root container not setted."
                     << " Init failed."
                     << endl;
        return false;
    }

    // context monitoring
    if( objrepr::RepresentationServer::instance()->currentContext() ){
        m_impl->slotContextLoaded();
    }

    objrepr::RepresentationServer::instance()->contextLoadingFinished.connect( boost::bind( & PrivateImplementation::slotContextLoaded, m_impl) );
    objrepr::RepresentationServer::instance()->contextUnloaded.connect( boost::bind( & PrivateImplementation::slotContextUnloaded, m_impl) );

    // async process of tasks
    m_impl->threadClientMaintenance = new std::thread( & PrivateImplementation::threadMaintenance, m_impl );

    VS_LOG_INFO << PRINT_HEADER
                << " init success"
                << " (root object name: [" << _settings.gdmRootObjectName << "])"
                << endl;
    m_impl->inited = true;
    return true;
}

std::vector<PVideoServerHandler> VideoServerClient::getVideoServerHandlers(){
    if( ! m_impl->inited ){
        VS_LOG_WARN << PRINT_HEADER << " not yet inited" << endl;
        return std::vector<PVideoServerHandler>();
    }

    m_impl->muVideoServerHandlers.lock();
    std::vector<PVideoServerHandler> out = m_impl->videoServerHandlers;
    m_impl->muVideoServerHandlers.unlock();

    return out;
}

const std::string & VideoServerClient::getLastError(){
    return m_impl->lastError;
}

void VideoServerClient::sendSignalClientMessage( string _msg ){
    emit signalClientMessage( _msg.c_str() );
}

void VideoServerClient::sendSignalNewServerDetected( TObjectId _serverId ){
    emit signalNewServerDetected( _serverId );
}

}
