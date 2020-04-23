
#include <iostream>

#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "cmd_source_connect.h"
#include "common_vars.h"

using namespace std;

namespace video_server_client{

CommandConnectSource::CommandConnectSource( SCommandServices * _commandServices ) :
    ACommand(_commandServices)
{

}

CommandConnectSource::~CommandConnectSource()
{

}

bool CommandConnectSource::serializeRequestTemplateMethodPart(){

}

bool CommandConnectSource::parseResponseTemplateMethodPart(){

}

string CommandConnectSource::execDerive(){

    if( request->sendOutcomingMessage( m_outcomingMessage ) ){
        return request->m_incomingMessage;
    }
    else{
        return string();
    }
}

bool CommandConnectSource::init( SInitialParams _params ){

    if( _params.sourceUrl.empty() ){
        return false;
    }

    Json::Value root;
    root[ "cmd_type" ] = "source";
    root[ "cmd_name" ] = "connect";
    root[ "url" ] = _params.sourceUrl;
    root[ "sensor_id" ] = (long long)_params.sensorId;

    Json::FastWriter writer;
    m_outcomingMessage = writer.write( root );

    m_params = _params;
    return true;
}

bool CommandConnectSource::parseResponse( const std::string & _msgBody ){

    Json::Reader reader;
    Json::Value parsedRecord;
    if( ! reader.parse( _msgBody.c_str(), parsedRecord, false ) ){

        cerr << common_vars::PRINT_HEADER
             << "Command: parse failed of [1] Reason: [2] "
             << _msgBody << " " << reader.getFormattedErrorMessages()
             << endl;

        m_lastError = reader.getFormattedErrorMessages();
        return false;
    }

    return true;
}

}
