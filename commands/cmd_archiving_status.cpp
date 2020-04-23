
#include <iostream>

#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "from_ms_common/system/logger.h"
#include "from_ms_common/common/ms_common_utils.h"
#include "video_server_handler.h"
#include "cmd_archiving_status.h"
#include "common_vars.h"
#include "common_utils.h"

namespace video_server_client{

using namespace std;
using namespace common_types;

const TArchivingId CommandArchivingStatus::ALL_ARCHIVINGS = "all_archivings";

CommandArchivingStatus::CommandArchivingStatus( SCommandServices * _commandServices )
    : ICommand(_commandServices)
{

}

bool CommandArchivingStatus::serializeRequestTemplateMethodPart(){
    Json::Value root;
    root[ "cmd_type" ] = "storage";
    root[ "cmd_name" ] = "status";
    root[ "archiving_id" ] = m_params.archivingId;

    Json::FastWriter writer;
    m_outcomingMsg = writer.write( root );

    return true;
}

bool CommandArchivingStatus::parseResponseTemplateMethodPart(){
    Json::Value parsedRecord;
    if( ! m_jsonReader.parse( m_incomingMsg.c_str(), parsedRecord, false ) ){
        VS_LOG_ERROR << "parse failed due to [" << m_jsonReader.getFormattedErrorMessages() << "]"
                     << " msg [" << m_incomingMsg << "]"
                     << endl;
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

        std::vector<SArchiveStatus> status = { m_status };
        m_commandServices->callbacks->updateArchivingStatus( status );
    }
}

}
