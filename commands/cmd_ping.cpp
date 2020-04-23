
#include "from_ms_common/system/logger.h"

#include "common_vars.h"
#include "cmd_ping.h"

namespace video_server_client {

using namespace std;

CommandPlayerPing::CommandPlayerPing( common_types::SCommandServices * _commandServices, PNetworkClient _network )
    : ICommand(_commandServices, _network)
{

}

bool CommandPlayerPing::serializeRequestTemplateMethodPart(){

    Json::Value rootRecord;
    rootRecord[ "user_id" ] = m_userIdToPlayer;
    rootRecord[ "cmd_type" ] = "service";
    rootRecord[ "cmd_name" ] = "ping";

    m_outcomingMsg = m_jsonWriter.write( rootRecord );

    return true;
}

bool CommandPlayerPing::parseResponseTemplateMethodPart(){

    Json::Value parsedRecord;
    if( ! m_jsonReader.parse( m_incomingMsg.c_str(), parsedRecord, false ) ){
        VS_LOG_ERROR << "parse failed due to [" << m_jsonReader.getFormattedErrorMessages() << "]"
                     << " msg [" << m_incomingMsg << "]"
                     << endl;
        return false;
    }

    m_commandServices->clientController->pongByPlayerCatched();

    const string code = parsedRecord["code"].asString();
    if( code != common_vars::PLAYER_CODE_NOT_REGISTERED ){
        const void * playerState;
        // parse from message
        m_commandServices->clientController->updatePlayer( playerState );
    }
    else{
        m_commandServices->clientController->registerInPlayer();
    }

    return true;
}

}










