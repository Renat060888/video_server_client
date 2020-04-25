#ifndef COMMAND_CONNECT_SOURCE_H
#define COMMAND_CONNECT_SOURCE_H

#include "i_command.h"
#include "../common_types_private.h"

namespace video_server_client{

class CommandConnectSource : public ICommand {
    // TODO: private CTOR/DTOR
    friend class VideoServerClient;
public:
    struct SInitialParams {
        SInitialParams()
            : sensorId(0)
        {}
        std::string sourceUrl;
        uint64_t sensorId;
    };

    CommandConnectSource( common_types::SCommandServices * _commandServices );
    ~CommandConnectSource();

    // request
    SInitialParams m_params;


private:
    virtual bool serializeRequestTemplateMethodPart() override;
    virtual bool parseResponseTemplateMethodPart() override;
};
using PCommandConnectSource = std::shared_ptr<CommandConnectSource>;

}

#endif // COMMAND_CONNECT_SOURCE_H
