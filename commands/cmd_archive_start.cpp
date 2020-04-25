
#include <iostream>

#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "from_ms_common/system/logger.h"
#include "from_ms_common/common/ms_common_utils.h"
#include "common_utils.h"
#include "video_server_handler.h"
#include "common_vars.h"
#include "cmd_archive_start.h"

namespace video_server_client{

using namespace std;
using namespace common_types;

CommandArchiveStart::CommandArchiveStart( SCommandServices * _commandServices )
    : ICommand(_commandServices)
{

}

bool CommandArchiveStart::serializeRequestTemplateMethodPart(){

    if( 0 == m_params.sensorId ){
        m_lastError = "sensor id is ZERO";
        return false;
    }
    // TODO: do
//    if( m_params.archivingName.empty() ){
//        m_lastError = "archiving name is EMPTY";
//        return false;
//    }

    Json::Value root;
    root[ "cmd_type" ] = "storage";
    root[ "cmd_name" ] = "start";
    root[ "sensor_id" ] = (unsigned long long)m_params.sensorId;
    root[ "archiving_name" ] = ( m_params.archivingName.empty() ? "name_not_defined" : m_params.archivingName );
    root[ "archiving_id" ] = m_params.archivingId;

    Json::FastWriter writer;
    m_outcomingMsg = writer.write( root );

    return true;
}

bool CommandArchiveStart::parseResponseTemplateMethodPart(){

    // parse
    Json::Value parsedRecord;
    if( ! m_jsonReader.parse( m_incomingMsg.c_str(), parsedRecord, false ) ){
        VS_LOG_ERROR << "parse failed due to [" << m_jsonReader.getFormattedErrorMessages() << "]"
                     << " msg [" << m_incomingMsg << "]"
                     << endl;
        return false;
    }

    // set
    Json::Value body = parsedRecord["body"];
    if( common_vars::incoming_commands::COMMAND_RESULT_SUCCESS == parsedRecord["response"].asString() ){
        SArchiveStatus state;
        state.archivingId = body["archiving_id"].asString();
        state.archiveState = common_utils::convertArchivingStateStr( body["archiving_state"].asString() );
        m_commandInitiator->addStatus( state, false );
        return true;
    }
    else{
        m_lastError = body["error_msg"].asString();
        return false;
    }
}

}
