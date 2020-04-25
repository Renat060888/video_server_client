
#include "from_ms_common/system/logger.h"

#include "common_vars.h"
#include "cmd_ping.h"

namespace video_server_client {

using namespace std;

static SServerState parseServerState( const Json::Value & _state ){

    SServerState server;
    return server;
}

static SSystemState parseSystemState( const Json::Value & _state ){

    SSystemState system;
    return system;
}

static std::vector<SArchiveStatus> parseArchiverStates( const Json::Value & _state ){

    std::vector<SArchiveStatus> state;
    return state;
}

static std::vector<SAnalyzeStatus> parseAnalyzerStates( const Json::Value & _state ){

    std::vector<SAnalyzeStatus> state;
    return state;
}

static std::vector<SAnalyticEvent> parseEvents( const Json::Value & _events ){

    std::vector<SAnalyticEvent> events;
    return events;
}

CommandPlayerPing::CommandPlayerPing( common_types::SCommandServices * _commandServices )
    : ICommand(_commandServices)
{

}

bool CommandPlayerPing::serializeRequestTemplateMethodPart(){

    Json::Value rootRecord;
    rootRecord[ "cmd_type" ] = "service";
    rootRecord[ "cmd_name" ] = "ping";

    m_outcomingMsg = m_jsonWriter.write( rootRecord );

    return true;
}

bool CommandPlayerPing::parseResponseTemplateMethodPart(){

    // parse
    Json::Value parsedRecord;
    if( ! m_jsonReader.parse( m_incomingMsg.c_str(), parsedRecord, false ) ){
        VS_LOG_ERROR << "parse failed due to [" << m_jsonReader.getFormattedErrorMessages() << "]"
                     << " msg [" << m_incomingMsg << "]"
                     << endl;
        return false;
    }

    // set
    m_commandServices->callbacks->pongCatched();
    m_commandServices->callbacks->updateServerState( parseServerState(parsedRecord["server_state"]) );
    m_commandServices->callbacks->updateSystemState( parseSystemState(parsedRecord["system_state"]) );
    m_commandServices->callbacks->updateArchivingStatus( parseArchiverStates(parsedRecord["archivers_state"]) );
    m_commandServices->callbacks->updateAnalyzeStatus( parseAnalyzerStates(parsedRecord["analyzers_state"]) );
    m_commandServices->callbacks->newEvent( parseEvents(parsedRecord["events"]) );

    return true;
}

}










