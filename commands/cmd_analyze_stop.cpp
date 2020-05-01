
#include <iostream>

#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "from_ms_common/system/logger.h"
#include "from_ms_common/common/ms_common_utils.h"
#include "video_server_handler.h"
#include "cmd_analyze_stop.h"
#include "common_vars.h"
#include "common_utils.h"

namespace video_server_client{

using namespace std;
using namespace common_types;

CommandAnalyzeStop::CommandAnalyzeStop( SCommandServices * _commandServices )
    : ICommand(_commandServices)
{

}

bool CommandAnalyzeStop::serializeRequestTemplateMethodPart(){
    Json::Value root;
    root[ "cmd_type" ] = "analyze";
    root[ "cmd_name" ] = "stop";
    root[ "sensor_id" ] = (unsigned long long)m_params.sensorId;
    root[ "processing_id" ] = m_params.processingId;
    root[ "destroy_current_session" ] = m_params.destroy;

    Json::FastWriter writer;
    m_outcomingMsg = writer.write( root );

    return true;
}

bool CommandAnalyzeStop::parseResponseTemplateMethodPart(){
    Json::Value parsedRecord;
    if( ! m_jsonReader.parse( m_incomingMsg.c_str(), parsedRecord, false ) ){
        VS_LOG_ERROR << "parse failed due to [" << m_jsonReader.getFormattedErrorMessages() << "]"
                     << " msg [" << m_incomingMsg << "]"
                     << endl;
        return false;
    }

    Json::Value body = parsedRecord["body"];
    if( common_vars::incoming_commands::COMMAND_RESULT_SUCCESS == parsedRecord["response"].asString() ){

        SAnalyzeStatus status;
        status.sensorId = body["sensor_id"].asInt64();
        status.analyzeState = common_utils::convertAnalyzeStateStr( body["analyze_status"].asString() );
        status.processingId = body["processing_id"].asString();
        status.processingName = body["processing_name"].asString();

        std::vector<SAnalyzeStatus> statuses = { status };
        m_commandServices->callbacks->updateAnalyzeStatus( statuses );
        return true;
    }
    else{
        m_lastError = body.asString();
        return false;
    }
}

}
