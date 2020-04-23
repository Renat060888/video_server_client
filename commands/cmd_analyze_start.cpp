
#include <iostream>

#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "video_server_handler.h"
#include "cmd_analyze_start.h"
#include "common_vars.h"
#include "common_utils.h"

using namespace std;

namespace video_server_client{

CommandAnalyzeStart::CommandAnalyzeStart( SCommandServices * _commandServices )
    : ACommand(_commandServices)
{

}

CommandAnalyzeStart::~CommandAnalyzeStart(){

}

bool CommandAnalyzeStart::serializeRequestTemplateMethodPart(){

}

bool CommandAnalyzeStart::parseResponseTemplateMethodPart(){

}

bool CommandAnalyzeStart::init( SInitialParams _params ){

    if( _params.resume ){
        if( _params.processingId.empty() ){
            m_lastError = "processing id is EMPTY for resume";
            return false;
        }

        Json::Value root;
        root[ "cmd_type" ] = "analyze";
        root[ "cmd_name" ] = "start";
        root[ "sensor_id" ] = (unsigned long long)_params.sensorId;
        root[ "profile_id" ] = (unsigned long long)_params.profileId;
        root[ "processing_name" ] = _params.processingName;
        root[ "processing_id" ] = _params.processingId;
        root[ "resume_current_session" ] = _params.resume;
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
        if( 0 == _params.sensorId ){
            m_lastError = "sensor id is ZERO";
            return false;
        }

        if( 0 == _params.profileId ){
            m_lastError = "profile id is ZERO";
            return false;
        }

        Json::Value array;
        for( auto & valuePair : _params.dpfLabelToObjreprClassinfo ){
            Json::Value element;
            element[ valuePair.first ] = valuePair.second;

            array.append( element );
        }

        Json::Value root;
        root[ "cmd_type" ] = "analyze";
        root[ "cmd_name" ] = "start";
        root[ "sensor_id" ] = (unsigned long long)_params.sensorId;
        root[ "profile_id" ] = (unsigned long long)_params.profileId;
        root[ "processing_name" ] = _params.processingName;
        root[ "processing_id" ] = _params.processingId;
        root[ "resume_current_session" ] = _params.resume;
        root[ "zone_x" ] = _params.zoneX;
        root[ "zone_y" ] = _params.zoneY;
        root[ "zone_w" ] = _params.zoneW;
        root[ "zone_h" ] = _params.zoneH;
        root[ "dpf_to_objrepr" ] = array;

        Json::FastWriter writer;
        m_outcomingMsg = writer.write( root );

        return true;
    }
}

string CommandAnalyzeStart::execDerive(){

//    if( ! m_commandServices->videoServerStatus->contextOpened ){
//        m_lastError = "context not opened";
//        return string();
//    }

    if( request->sendOutcomingMessage( m_outcomingMessage ) ){
        return request->getIncomingMessage();
    }
    else{
        return string();
    }
}

bool CommandAnalyzeStart::parseResponse( const std::string & _msgBody ){

    Json::Reader reader;
    Json::Value parsedRecord;
    if( ! reader.parse( _msgBody.c_str(), parsedRecord, false ) ){

        cerr << common_vars::PRINT_HEADER
             << "Command: parse failed of [1] Reason: [2] "
             << _msgBody << " " << reader.getFormattedErrorMessages()
             << endl;

        m_lastError = "";
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

