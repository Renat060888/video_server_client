
#include <iostream>

#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "from_ms_common/system/logger.h"
#include "from_ms_common/common/ms_common_utils.h"
#include "video_server_handler.h"
#include "cmd_archive_stop.h"
#include "common_vars.h"
#include "common_utils.h"

namespace video_server_client{

using namespace std;
using namespace common_types;

CommandArchiveStop::CommandArchiveStop( SCommandServices * _commandServices )
    : ICommand(_commandServices)
{

}

bool CommandArchiveStop::serializeRequestTemplateMethodPart(){

    Json::Value root;
    root[ "cmd_type" ] = "storage";
    root[ "cmd_name" ] = "stop";
    root[ "archiving_id" ] = m_params.archivingId;
    root[ "destroy_current_session" ] = m_params.destroy;

    Json::FastWriter writer;
    m_outcomingMsg = writer.write( root );

    return true;
}

bool CommandArchiveStop::parseResponseTemplateMethodPart(){

    Json::Value parsedRecord;
    if( ! m_jsonReader.parse( m_incomingMsg.c_str(), parsedRecord, false ) ){
        VS_LOG_ERROR << "parse failed due to [" << m_jsonReader.getFormattedErrorMessages() << "]"
                     << " msg [" << m_incomingMsg << "]"
                     << endl;
        return false;
    }

    Json::Value body = parsedRecord["body"];
    if( common_vars::incoming_commands::COMMAND_RESULT_SUCCESS == parsedRecord["response"].asString() ){

        SArchiveStatus status;
        status.archivingId = body["archiving_id"].asString();
        status.archiveState = common_utils::convertArchivingStateStr( body["archiving_state"].asString() );

        std::vector<SArchiveStatus> statuses = { status };
        m_commandServices->callbacks->updateArchivingStatus( statuses );
        return true;
    }
    else{
        m_lastError = body["error_msg"].asString();
        return false;
    }
}

}

