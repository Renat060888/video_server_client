
#include "from_ms_common/system/logger.h"

#include "common_vars.h"
#include "common_utils.h"
#include "cmd_ping.h"

namespace video_server_client {

using namespace std;

static SServerState parseServerState( const Json::Value & _state ){

    SServerState server;
    server.currentContextId = _state["ctx_id"].asInt64();
    return server;
}

static SSystemState parseSystemState( const Json::Value & _state ){

    const int32_t ramAvailableMb = _state["core"]["ram_available_mb"].asInt();
    const int32_t ramTotalMb = _state["core"]["ram_total_mb"].asInt();

    SSystemState system;
    system.cpuUsagePercent = _state["core"]["cpu_load_percent"].asInt();
    system.systemMemoryUsagePercent = ( (float)(ramTotalMb - ramAvailableMb) / (float)ramTotalMb ) * 100.0;
    system.gpuUsagePercent = _state["core"]["gpu_load_percent"].asInt();
    system.gpuMemoryUsagePercent = _state["core"]["memory_used_percent"].asInt();
    return system;
}

static std::vector<SArchiveStatus> parseArchiverStates( const Json::Value & _statesFromServer ){

    std::vector<SArchiveStatus> states;
    for( Json::ArrayIndex i = 0; i < _statesFromServer.size(); i++ ){
        const Json::Value & arrElement = _statesFromServer[ i ];

        SArchiveStatus state;
        state.sensorId = arrElement["sensor_id"].asInt64();
        state.archivingId = arrElement["archiving_id"].asString();
        state.archiveState = common_utils::convertArchivingStateStr( arrElement["archiver_status"].asString() );

        states.push_back( state);
    }
    return states;
}

static std::vector<SAnalyzeStatus> parseAnalyzerStates( const Json::Value & _statesFromServer ){

    std::vector<SAnalyzeStatus> states;
    for( Json::ArrayIndex i = 0; i < _statesFromServer.size(); i++ ){
        const Json::Value & arrElement = _statesFromServer[ i ];

        SAnalyzeStatus state;
        state.sensorId = arrElement["sensor_id"].asInt64();
        state.profileId = arrElement["profile_id"].asInt64();
        state.processingId = arrElement["processing_id"].asString();
        state.analyzeState = common_utils::convertAnalyzeStateStr( arrElement["analyzer_status"].asString() );

        states.push_back( state);
    }
    return states;
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

//    cout << " pong: " << m_incomingMsg << endl;

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










