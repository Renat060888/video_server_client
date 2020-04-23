
#include <iostream>

#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "from_ms_common/system/logger.h"
#include "from_ms_common/common/ms_common_utils.h"
#include "video_server_handler.h"
#include "cmd_analyze_start.h"
#include "common_vars.h"
#include "common_utils.h"

namespace video_server_client{

using namespace std;
using namespace common_types;

CommandAnalyzeStart::CommandAnalyzeStart( SCommandServices * _commandServices )
    : ICommand(_commandServices)
{

}

CommandAnalyzeStart::~CommandAnalyzeStart(){

}

bool CommandAnalyzeStart::serializeRequestTemplateMethodPart(){
    if( m_params.resume ){
        if( m_params.processingId.empty() ){
            m_lastError = "processing id is EMPTY for resume";
            return false;
        }

        Json::Value root;
        root[ "cmd_type" ] = "analyze";
        root[ "cmd_name" ] = "start";
        root[ "sensor_id" ] = (unsigned long long)m_params.sensorId;
        root[ "profile_id" ] = (unsigned long long)m_params.profileId;
        root[ "processing_name" ] = m_params.processingName;
        root[ "processing_id" ] = m_params.processingId;
        root[ "resume_current_session" ] = m_params.resume;
        root[ "zone_x" ] = 0;
        root[ "zone_y" ] = 0;
        root[ "zone_w" ] = 0;
        root[ "zone_h" ] = 0;
        Json::Value nullArray;
        root[ "dpf_to_objrepr" ] = nullArray;

        Json::FastWriter writer;
        m_outcomingMsg = writer.write( root );

        return true;
    }
    else{
        if( 0 == m_params.sensorId ){
            m_lastError = "sensor id is ZERO";
            return false;
        }

        if( 0 == m_params.profileId ){
            m_lastError = "profile id is ZERO";
            return false;
        }

        Json::Value array;
        for( auto & valuePair : m_params.dpfLabelToObjreprClassinfo ){
            Json::Value element;
            element[ valuePair.first ] = valuePair.second;

            array.append( element );
        }

        Json::Value root;
        root[ "cmd_type" ] = "analyze";
        root[ "cmd_name" ] = "start";
        root[ "sensor_id" ] = (unsigned long long)m_params.sensorId;
        root[ "profile_id" ] = (unsigned long long)m_params.profileId;
        root[ "processing_name" ] = m_params.processingName;
        root[ "processing_id" ] = m_params.processingId;
        root[ "resume_current_session" ] = m_params.resume;
        root[ "zone_x" ] = m_params.zoneX;
        root[ "zone_y" ] = m_params.zoneY;
        root[ "zone_w" ] = m_params.zoneW;
        root[ "zone_h" ] = m_params.zoneH;
        root[ "dpf_to_objrepr" ] = array;

        Json::FastWriter writer;
        m_outcomingMsg = writer.write( root );

        return true;
    }
}

bool CommandAnalyzeStart::parseResponseTemplateMethodPart(){
    Json::Value parsedRecord;
    if( ! m_jsonReader.parse( m_incomingMsg.c_str(), parsedRecord, false ) ){
        VS_LOG_ERROR << "parse failed due to [" << m_jsonReader.getFormattedErrorMessages() << "]"
                     << " msg [" << m_incomingMsg << "]"
                     << endl;
        return false;
    }

    Json::Value body = parsedRecord["body"];
    if( common_vars::incoming_commands::COMMAND_RESULT_SUCCESS == parsedRecord["response"].asString() ){
//        SAnalyzeStatus status;
//        status.sensorId = body["sensor_id"].asInt64();
//        status.analyzeState = common_utils::convertAnalyzeStateStr( body["analyze_state"].asString() );
//        status.processingId = body["processing_id"].asString();
//        status.processingName = body["processing_name"].asString();
//        m_commandServices->handler->addAnalyzeStatus( status );

        m_processingId = body["processing_id"].asString();
        m_analyzeState = common_utils::convertAnalyzeStateStr( body["analyze_state"].asString() );
        return true;
    }
    else{
        m_lastError = body["error_msg"].asString();
        return false;
    }
}

}

