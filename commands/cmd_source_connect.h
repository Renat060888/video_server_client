#ifndef COMMAND_CONNECT_SOURCE_H
#define COMMAND_CONNECT_SOURCE_H

#include "i_command.h"
#include "common_types_private.h"

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

    bool init( SInitialParams _params );


protected:
    virtual bool parseResponse( const std::string & _msgBody ) override;
    virtual std::string execDerive() override;


private:
    virtual bool serializeRequestTemplateMethodPart() override;
    virtual bool parseResponseTemplateMethodPart() override;

    SInitialParams m_params;
};
using PCommandConnectSource = std::shared_ptr<CommandConnectSource>;

}

#endif // COMMAND_CONNECT_SOURCE_H
