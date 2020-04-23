
#include <iostream>

#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "from_ms_common/system/logger.h"
#include "from_ms_common/common/ms_common_utils.h"
#include "cmd_source_connect.h"
#include "common_vars.h"

namespace video_server_client{

using namespace std;
using namespace common_types;

CommandConnectSource::CommandConnectSource( SCommandServices * _commandServices ) :
    ICommand(_commandServices)
{

}

CommandConnectSource::~CommandConnectSource()
{

}

bool CommandConnectSource::serializeRequestTemplateMethodPart(){
    if( m_params.sourceUrl.empty() ){
        return false;
    }

    Json::Value root;
    root[ "cmd_type" ] = "source";
    root[ "cmd_name" ] = "connect";
    root[ "url" ] = m_params.sourceUrl;
    root[ "sensor_id" ] = (long long)m_params.sensorId;

    Json::FastWriter writer;
    m_outcomingMsg = writer.write( root );

    return true;
}

bool CommandConnectSource::parseResponseTemplateMethodPart(){
    Json::Value parsedRecord;
    if( ! m_jsonReader.parse( m_incomingMsg.c_str(), parsedRecord, false ) ){
        VS_LOG_ERROR << "parse failed due to [" << m_jsonReader.getFormattedErrorMessages() << "]"
                     << " msg [" << m_incomingMsg << "]"
                     << endl;
        return false;
    }

    // TODO: do
}

}
