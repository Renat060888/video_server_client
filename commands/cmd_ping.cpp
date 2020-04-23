
#include "from_ms_common/system/logger.h"

#include "common_vars.h"
#include "cmd_ping.h"

namespace video_server_client {

using namespace std;

CommandPlayerPing::CommandPlayerPing( common_types::SCommandServices * _commandServices )
    : ICommand(_commandServices)
{

}

bool CommandPlayerPing::serializeRequestTemplateMethodPart(){

    Json::Value rootRecord;
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

    m_commandServices->callbacks->pongCatched();

    const string code = parsedRecord["code"].asString();




    return true;
}

}










