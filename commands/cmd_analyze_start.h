#ifndef COMMAND_START_ANALYZE_H
#define COMMAND_START_ANALYZE_H

#include <list>
#include <map>

#include "i_command.h"
#include "common_types_private.h"

namespace video_server_client{

class CommandAnalyzeStart : public ICommand {
    // TODO: private CTOR/DTOR
    friend class VideoServerClient;
public:
    struct SInitialParams {
        SInitialParams()
            : sensorId(0)
            , profileId(0)
            , zoneX(0)
            , zoneY(0)
            , zoneW(0)
            , zoneH(0)
            , resume(false)
            , analyzeState(EAnalyzeState::UNDEFINED)
        {}
        uint64_t sensorId;
        uint64_t profileId;
        std::list<uint64_t> classinfosId;
        std::map<std::string, std::string> dpfLabelToObjreprClassinfo;
        int32_t zoneX;
        int32_t zoneY;
        int32_t zoneW;
        int32_t zoneH;

        // for 'resume' command
        std::string processingName;
        bool resume;

        // creating from status
        std::string processingId;
        EAnalyzeState analyzeState;
    };    

    CommandAnalyzeStart( common_types::SCommandServices * _commandServices );
    ~CommandAnalyzeStart();

    // request
    SInitialParams m_params;

    // NOTE: response from server
    std::string m_processingId;
    EAnalyzeState m_analyzeState;


private:
    virtual bool serializeRequestTemplateMethodPart() override;
    virtual bool parseResponseTemplateMethodPart() override;
};
using PCommandAnalyzeStart = std::shared_ptr<CommandAnalyzeStart>;

}

#endif // COMMAND_START_ANALYZE_H

