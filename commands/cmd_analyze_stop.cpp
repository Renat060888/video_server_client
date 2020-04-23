
#include <iostream>

#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "video_server_handler.h"
#include "cmd_analyze_stop.h"
#include "common_vars.h"
#include "common_utils.h"

using namespace std;

namespace video_server_client{

CommandAnalyzeStop::CommandAnalyzeStop( SCommandServices * _commandServices )
    : ACommand(_commandServices)
{

}

bool CommandAnalyzeStop::serializeRequestTemplateMethodPart(){

}

bool CommandAnalyzeStop::parseResponseTemplateMethodPart(){

}

bool CommandAnalyzeStop::parseResponse( const std::string & _msgBody ){

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

        SAnalyzeStatus status;
        status.sensorId = body["sensor_id"].asInt64();
        status.analyzeState = common_utils::convertAnalyzeStateStr( body["analyze_state"].asString() );
        status.processingId = body["processing_id"].asString();
        status.processingName = body["processing_name"].asString();

        m_commandServices->handler->addAnalyzeStatus( status, true ); // TODO: destroy=true - WTF?
        return true;
    }
    else{
        m_lastError = body.asString();
        return false;
    }
}

std::string CommandAnalyzeStop::execDerive(){

//    if( ! m_commandServices->videoServerStatus->contextOpened ){
//        m_lastError = "context not opened";
//        return string();
//    }

    if( request->sendOutcomingMessage( m_outcomingMessage ) ){
        return request->m_incomingMessage;
    }
    else{
        return string();
    }
}

bool CommandAnalyzeStop::init( SInitialParams _params ){

    Json::Value root;
    root[ "cmd_type" ] = "analyze";
    root[ "cmd_name" ] = "stop";
    root[ "sensor_id" ] = (unsigned long long)_params.sensorId;
    root[ "processing_id" ] = _params.processingId;
    root[ "destroy_current_session" ] = _params.destroy;

    Json::FastWriter writer;
    m_outcomingMessage = writer.write( root );

    return true;
}

}
