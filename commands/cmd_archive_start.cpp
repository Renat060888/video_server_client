
#include <iostream>

#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "video_server_handler.h"
#include "common_vars.h"
#include "common_utils.h"
#include "cmd_archive_start.h"

namespace video_server_client{

using namespace std;

CommandArchiveStart::CommandArchiveStart( SCommandServices * _commandServices )
    : ACommand(_commandServices)
{

}

bool CommandArchiveStart::serializeRequestTemplateMethodPart(){

}

bool CommandArchiveStart::parseResponseTemplateMethodPart(){

}

bool CommandArchiveStart::init( SInitialParams _params ){

    if( 0 == _params.sensorId ){
        m_lastError = "sensor id is ZERO";
        return false;
    }

    if( _params.archivingName.empty() ){
        m_lastError = "archiving name is EMPTY";
        return false;
    }

    Json::Value root;
    root[ "cmd_type" ] = "storage";
    root[ "cmd_name" ] = "start";
    root[ "sensor_id" ] = (unsigned long long)_params.sensorId;
    root[ "archiving_name" ] = _params.archivingName;
    root[ "archiving_id" ] = _params.archivingId;

    Json::FastWriter writer;
    m_outcomingMessage = writer.write( root );

    return true;
}

bool CommandArchiveStart::parseResponse( const std::string & _msgBody ){

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

//    SArchiveStatus status;
//    status.sensorId = body["sensor_id"].asInt64();
//    status.archivingName = body["archiving_name"].asString();
//    status.archivingId = body["archiving_id"].asString();
//    status.archiveState = common_utils::convertArchivingStateStr( body["archiving_state"].asString() );
//    m_commandServices->handler->addArchivingStatus( status );

    if( common_vars::incoming_commands::COMMAND_RESULT_SUCCESS == parsedRecord["response"].asString() ){
        m_archivingId = body["archiving_id"].asString();
        m_archiveState = common_utils::convertArchivingStateStr( body["archiving_state"].asString() );
        return true;
    }
    else{
        m_lastError = body["error_msg"].asString();
        return false;
    }
}

std::string CommandArchiveStart::execDerive(){

    if( request->sendOutcomingMessage( m_outcomingMessage ) ){
        return request->getIncomingMessage();
    }
    else{
        return string();
    }
}

}
