
#include <iostream>

#include <QDateTime>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QHostInfo>
#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>

#include "video_server_handler.h"
#include "cmd_analyze_status.h"
#include "common_vars.h"
#include "common_utils.h"

namespace video_server_client{

using namespace std;

const TProcessingId CommandAnalyzeStatus::ALL_PROCESSING = "all_processings";

static string getLocalIpAddress(){

    string ipAddress = "not_found";
    foreach( const QHostAddress & address, QNetworkInterface::allAddresses() ){

        if( address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost) ){
            ipAddress = address.toString().toStdString();
            break;
        }
    }

    return ipAddress;
}

CommandAnalyzeStatus::CommandAnalyzeStatus( SCommandServices * _commandServices )
    : ACommand(_commandServices)
    , m_localHostname( QHostInfo::localHostName().toStdString() )
    , m_localIpAddress( getLocalIpAddress() )
{

}

bool CommandAnalyzeStatus::serializeRequestTemplateMethodPart(){

}

bool CommandAnalyzeStatus::parseResponseTemplateMethodPart(){

}

bool CommandAnalyzeStatus::parseResponse( const std::string & _msgBody ){

    try{
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
            Json::Value statusesRecords = body["analyzers_status"];
            for( Json::ArrayIndex i = 0; i < statusesRecords.size(); i++ ){
                Json::Value record = statusesRecords[ i ];

                m_status.sensorId = record["sensor_id"].asUInt64();
                m_status.analyzeState = common_utils::convertAnalyzeStateStr( record["state"].asString() );
                m_status.processingId = record["processing_id"].asString();
                m_status.processingName = record["processing_name"].asString();
                m_status.profileId = record["profile_id"].asUInt64();

                m_commandServices->handler->addAnalyzeStatus( m_status );
            }
        }
        else{
            m_lastError = body.asString();
            return false;
        }
    }catch( Json::Exception & _ex ){
        cerr << "CommandAnalyzeStatus::parseResponse fail, reason: " << _ex.what() << endl;
        return false;
    }

    return true;
}

bool CommandAnalyzeStatus::init( SInitialParams _params ){

    Json::Value root;
    root[ "cmd_type" ] = "analyze";
    root[ "cmd_name" ] = "status";
    root[ "processing_id" ] = _params.processingId;
    root[ "sensor_id" ] = (unsigned long long)_params.sensorId;
    root[ "user_ip" ] = m_localIpAddress;
    root[ "user_hostname" ] = m_localHostname;

    Json::FastWriter writer;
    m_outcomingMessage = writer.write( root );

    return true;
}

string CommandAnalyzeStatus::execDerive(){    

    if( request->sendOutcomingMessage( m_outcomingMessage ) ){
        return request->m_incomingMessage;
    }
    else{
        return string();
    }

    return string();
}

}
