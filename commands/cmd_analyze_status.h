#ifndef CMD_ANALYZE_STATUS_H
#define CMD_ANALYZE_STATUS_H

#include "i_command.h"
#include "common_types_private.h"

namespace video_server_client{

class CommandAnalyzeStatus : public ICommand
{
public:
    CommandAnalyzeStatus( common_types::SCommandServices * _commandServices );

    static constexpr uint64_t ALL_SENSORS = 0;
    static const TProcessingId ALL_PROCESSING;

    struct SInitialParams {
        SInitialParams()
            : sensorId(0)
        {}

        TProcessingId processingId;
        uint64_t sensorId;
    };   

    bool init( SInitialParams _params );

    SAnalyzeStatus m_status;


protected:
    virtual bool parseResponse( const std::string & _msgBody ) override;
    virtual std::string execDerive() override;


private:
    virtual bool serializeRequestTemplateMethodPart() override;
    virtual bool parseResponseTemplateMethodPart() override;

    const std::string m_localHostname;
    const std::string m_localIpAddress;
};
using PCommandAnalyzeStatus = std::shared_ptr<CommandAnalyzeStatus>;

}

#endif // CMD_ANALYZE_STATUS_H
