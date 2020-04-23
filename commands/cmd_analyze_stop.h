#ifndef CMD_STOP_ANALYZE_H
#define CMD_STOP_ANALYZE_H

#include "i_command.h"
#include "common_types_private.h"

namespace video_server_client{

class CommandAnalyzeStop : public ICommand {
    // TODO: private CTOR/DTOR
    friend class VideoServerClient;
public:
    struct SInitialParams {
        SInitialParams()
            : sensorId(0)
            , destroy(false)
        {}
        TProcessingId processingId;
        uint64_t sensorId;
        bool destroy;
    };

    CommandAnalyzeStop( common_types::SCommandServices * _commandServices );
    ~CommandAnalyzeStop(){}

    // request
    SInitialParams m_params;


private:
    virtual bool serializeRequestTemplateMethodPart() override;
    virtual bool parseResponseTemplateMethodPart() override;

};
using PCommandAnalyzeStop = std::shared_ptr<CommandAnalyzeStop>;

}

#endif // CMD_STOP_ANALYZE_H
