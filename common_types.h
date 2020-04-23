#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <vector>
#include <future>

namespace video_server_client{

using TSensorId = uint64_t;
using TObjectId = uint64_t;
using TProcessingId = std::string;
using TArchivingId = std::string;
using TTimeRange = std::pair<int64_t, int64_t>;

// -------------------------------
// payload
// -------------------------------
struct STrackedObject {
    std::string name;
    uint64_t objectId;
    uint64_t classId;
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
};

struct SAnalyticEvent {

    void clear(){
        m_sensorId = 0;
        m_totalCount = 0;
        m_processingId.clear();
    }

    uint64_t m_sensorId;
    TProcessingId m_processingId;
    int m_totalCount;
    std::vector<STrackedObject> m_newObjects;
    std::vector<STrackedObject> m_changedObjects;
    std::vector<STrackedObject> m_disapprearedObjects;
};
using PConstAnalyticEvent = std::shared_ptr<const SAnalyticEvent>;
using PAnalyticEvent = std::shared_ptr<SAnalyticEvent>;

// -------------------------------
// types
// -------------------------------
enum class EServerRole {
    RETRANSLATION,
    ARCHIVING,
    ANALYZE,
    UNDEFINED
};

// NOTE: must be equal in video_server
enum class EArchiveState {
    UNAVAILABLE,
    PREPARING,
    READY,
    ACTIVE,
    CRUSHED,
    UNDEFINED
};

// NOTE: must be equal in video_server
enum class EAnalyzeState {
    UNAVAILABLE,
    PREPARING,
    READY,
    ACTIVE,
    CRUSHED,
    UNDEFINED
};

// -------------------------------
// internal structures
// -------------------------------
struct SSystemState {
    float cpuUsagePercent;
    int32_t systemMemoryUsagePercent;
    int32_t gpuUsagePercent;
    int32_t gpuMemoryUsagePercent;
};

struct SServerState {
    TObjectId objreprId;
    EServerRole role;
    uint32_t currentContextId;
};

struct SVideoServerStatus {
    SVideoServerStatus(){
        clear();
    }

    void clear(){
        contextOpened = false;
        lastServerPongMillisec = 0;
        available = false;
        connectionEstablished = false;
        cpuUsagePercent = 0;
        systemMemoryUsagePercent = 0;
        gpuUsagePercent = 0;
        gpuMemoryUsagePercent = 0;
        role = EServerRole::UNDEFINED;
        currentContextId = 0;
    }

    bool contextOpened;
    int64_t lastServerPongMillisec;
    bool available;
    bool connectionEstablished;

    // TODO:
    float cpuUsagePercent;
    int32_t systemMemoryUsagePercent;
    int32_t gpuUsagePercent;
    int32_t gpuMemoryUsagePercent;

    TObjectId objreprId;
    EServerRole role;
    uint32_t currentContextId;
};
using PConstVideoServerStatus = std::shared_ptr<const SVideoServerStatus>;
using PVideoServerStatus = std::shared_ptr<SVideoServerStatus>;


//


struct SArchiveStatus {
    SArchiveStatus()
    {
        clear();
    }

    void clear(){
        sensorId = 0;
        archiveState = EArchiveState::UNDEFINED;
    }

    EArchiveState archiveState;

    // TODO: deprecated
    uint64_t sensorId;
    std::string archivingId;
    std::string archivingName;
};

struct SAnalyzeStatus {
    SAnalyzeStatus()
    {
        clear();
    }

    void clear(){
        sensorId = 0;
        analyzeState = EAnalyzeState::UNDEFINED;
        profileId = 0;
    }

    EAnalyzeState analyzeState;

    // TODO: deprecated
    uint64_t sensorId;
    std::string processingId;
    std::string processingName;
    uint64_t profileId;
};

struct SSensorRecord {
    SSensorRecord()
        : sensorId(0)
    {}
    TObjectId sensorId;
    std::vector<TTimeRange> recordSessionsMillisec;
};

struct SAnalyzeLaunchParams {
    SAnalyzeLaunchParams()
        : sensorId(0)
    {}

    uint64_t sensorId;
    std::string processingId;
};







}

#endif // COMMON_TYPES_H
