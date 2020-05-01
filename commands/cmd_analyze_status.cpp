
#include <iostream>

#include <QDateTime>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QHostInfo>
#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "from_ms_common/system/logger.h"
#include "from_ms_common/common/ms_common_utils.h"
#include "video_server_handler.h"
#include "cmd_analyze_status.h"
#include "common_vars.h"
#include "common_utils.h"

namespace video_server_client{

using namespace std;
using namespace common_types;

const TProcessingId CommandAnalyzeStatus::ALL_PROCESSING = "all_processings";

CommandAnalyzeStatus::CommandAnalyzeStatus( SCommandServices * _commandServices )
    : ICommand(_commandServices)
    , m_localHostname( QHostInfo::localHostName().toStdString() )
    , m_localIpAddress( ::common_utils::getIpAddressStr() )
{

}

bool CommandAnalyzeStatus::serializeRequestTemplateMethodPart(){
    Json::Value root;
    root[ "cmd_type" ] = "analyze";
    root[ "cmd_name" ] = "status";
    root[ "processing_id" ] = m_params.processingId;
    root[ "sensor_id" ] = (unsigned long long)m_params.sensorId;
    root[ "user_ip" ] = m_localIpAddress;
    root[ "user_hostname" ] = m_localHostname;

    Json::FastWriter writer;
    m_outcomingMsg = writer.write( root );

    return true;
}

bool CommandAnalyzeStatus::parseResponseTemplateMethodPart(){
    Json::Value parsedRecord;
    if( ! m_jsonReader.parse( m_incomingMsg.c_str(), parsedRecord, false ) ){
        VS_LOG_ERROR << "parse failed due to [" << m_jsonReader.getFormattedErrorMessages() << "]"
                     << " msg [" << m_incomingMsg << "]"
                     << endl;
        return false;
    }

    m_commandServices->callbacks->analyzersInUpdate( true );

    Json::Value body = parsedRecord["body"];
    Json::Value statusesRecords = body["analyzers_status"];
    for( Json::ArrayIndex i = 0; i < statusesRecords.size(); i++ ){
        Json::Value record = statusesRecords[ i ];

        m_status.sensorId = record["sensor_id"].asUInt64();
        m_status.analyzeState = common_utils::convertAnalyzeStateStr( record["state"].asString() );
        m_status.processingId = record["processing_id"].asString();
        m_status.processingName = record["processing_name"].asString();
        m_status.profileId = record["profile_id"].asUInt64();

        std::vector<SAnalyzeStatus> status = { m_status };
        m_commandServices->callbacks->updateAnalyzeStatus( status );
    }

    m_commandServices->callbacks->analyzersInUpdate( false );
    return true;
}

}
