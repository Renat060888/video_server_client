
#include <objrepr/reprServer.h>
#include <objrepr/config_reader.h>

#include "from_ms_common/system/logger.h"
#include "video_server_client.h"

using namespace std;

int main( int argc, char ** argv, char ** env ){

    // TODO: from MS-Common
    const bool configured = objrepr::RepresentationServer::instance()->configure( "/home/renat/.config/drone_client/objrepr_cfg.xml" );
    if( ! configured ){
        VS_LOG_CRITICAL << "objrepr Can't configure by: [1], reason: [2] " << "cfg path" << " " << objrepr::RepresentationServer::instance()->errString() << endl;
        return false;
    }

    const bool launched = objrepr::RepresentationServer::instance()->launch();
    if( ! launched ){
        VS_LOG_CRITICAL << "objrepr Can't launch, reason: " << objrepr::RepresentationServer::instance()->errString() << endl;
        return false;
    }

    const bool opened = objrepr::RepresentationServer::instance()->setCurrentContext( "video_server_archiving_test" );
    if( ! opened ){
        const string lastError = objrepr::RepresentationServer::instance()->errString();
        VS_LOG_CRITICAL << "objrepr context open fail, reason: " << objrepr::RepresentationServer::instance()->errString() << endl;
        return false;
    }
    //

//    objrepr::ConfigReader & cr = objrepr::ConfigReader::singleton();
//    cout << cr.get().amqpServerPort << endl;
//    cout << cr.get().amqpServerHost << endl;

    video_server_client::VideoServerClient * vsc = video_server_client::VideoServerClient::getInstance();
    video_server_client::VideoServerClient::SInitSettings settings;
    settings.gdmRootObjectName = "Система анализа видео";
    assert( vsc->init(settings) );

    while( true ){
        //dummy
        this_thread::sleep_for( chrono::milliseconds(10) );
    }

    return 0;
}
