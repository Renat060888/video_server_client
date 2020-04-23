
#include <iostream>

#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "video_server_handler.h"
#include "cmd_archive_stop.h"
#include "common_vars.h"
#include "common_utils.h"

namespace video_server_client{

using namespace std;

CommandArchiveStop::CommandArchiveStop( SCommandServices * _commandServices )
    : ACommand(_commandServices)
{

}

bool CommandArchiveStop::serializeRequestTemplateMethodPart(){

}

bool CommandArchiveStop::parseResponseTemplateMethodPart(){

}

bool CommandArchiveStop::init( SInitialParams _params ){

    Json::Value root;
    root[ "cmd_type" ] = "storage";
    root[ "cmd_name" ] = "stop";
    root[ "archiving_id" ] = _params.archivingId;
    root[ "destroy_current_session" ] = _params.destroy;

    Json::FastWriter writer;
    m_outcomingMessage = writer.write( root );

    return true;
}

bool CommandArchiveStop::parseResponse( const std::string & _msgBody ){

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

        SArchiveStatus status;
        status.archivingId = body["archiving_id"].asString();
        status.archiveState = common_utils::convertArchivingStateStr( body["archiving_state"].asString() );

        m_commandServices->handler->addArchivingStatus( status, true );
        return true;
    }
    else{
        m_lastError = body["error_msg"].asString();
        return false;
    }
}

std::string CommandArchiveStop::execDerive(){

    cout << "========================= CommandArchiveStop()" << endl;

    if( request->sendOutcomingMessage( m_outcomingMessage ) ){
        return request->m_incomingMessage;
    }
    else{
        return string();
    }
}


}

