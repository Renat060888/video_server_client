
#include <iostream>

#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "video_server_handler.h"
#include "cmd_archiving_status.h"
#include "common_vars.h"
#include "common_utils.h"

namespace video_server_client{

using namespace std;

const TArchivingId CommandArchivingStatus::ALL_ARCHIVINGS = "all_archivings";

CommandArchivingStatus::CommandArchivingStatus( SCommandServices * _commandServices )
    : ACommand(_commandServices)
{

}

bool CommandArchivingStatus::serializeRequestTemplateMethodPart(){

}

bool CommandArchivingStatus::parseResponseTemplateMethodPart(){

}

bool CommandArchivingStatus::init( SInitialParams _params ){

    Json::Value root;
    root[ "cmd_type" ] = "storage";
    root[ "cmd_name" ] = "status";
    root[ "archiving_id" ] = _params.archivingId;

    Json::FastWriter writer;
    m_outcomingMessage = writer.write( root );

    return true;
}

bool CommandArchivingStatus::parseResponse( const std::string & _msgBody ){

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

    Json::Value statusesRecords = body["archivers_status"];
    for( Json::ArrayIndex i = 0; i < statusesRecords.size(); i++ ){
        Json::Value record = statusesRecords[ i ];

        m_status.sensorId = record["sensor_id"].asUInt64();
        m_status.archivingId = record["archiving_id"].asString();
        m_status.archivingName = record["archiving_name"].asString();
        m_status.archiveState = common_utils::convertArchivingStateStr( record["state"].asString() );

        m_commandServices->handler->addArchivingStatus( m_status );
    }

    return true;
}

std::string CommandArchivingStatus::execDerive(){

    if( request->sendOutcomingMessage( m_outcomingMessage ) ){
        return request->m_incomingMessage;
    }
    else{
        return string();
    }
}

}
