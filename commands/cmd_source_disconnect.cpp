
#include <iostream>

#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "cmd_source_disconnect.h"
#include "common_vars.h"

using namespace std;

namespace video_server_client{

CommandDisconnectSource::CommandDisconnectSource( SCommandServices * _commandServices ) :
    ACommand(_commandServices)
{

}

bool CommandDisconnectSource::serializeRequestTemplateMethodPart(){

}

bool CommandDisconnectSource::parseResponseTemplateMethodPart(){

}

bool CommandDisconnectSource::parseResponse( const std::string & _msgBody ){

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

    return true;
}

std::string CommandDisconnectSource::execDerive(){

    if( request->sendOutcomingMessage( m_outcomingMessage ) ){
        return request->m_incomingMessage;
    }
    else{
        return string();
    }
}

bool CommandDisconnectSource::init( SInitialParams _params ){

    Json::Value root;
    root[ "cmd_type" ] = "source";
    root[ "cmd_name" ] = "disconnect";
    root[ "url" ] = _params.sourceUrl;
    root[ "sensor_id" ] = (long long)_params.sensorId;

    Json::FastWriter writer;
    m_outcomingMessage = writer.write( root );

    return true;
}

}
