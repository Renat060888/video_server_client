#ifndef CMD_DISCONNECT_SOURCE_H
#define CMD_DISCONNECT_SOURCE_H

#include "i_command.h"
#include "common_types_private.h"

namespace video_server_client{

class CommandDisconnectSource : public ICommand {
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

    CommandDisconnectSource( common_types::SCommandServices * _commandServices );
    ~CommandDisconnectSource(){}

    bool init( SInitialParams _params );


protected:
    virtual bool parseResponse( const std::string & _msgBody ) override;
    virtual std::string execDerive() override;


private:
    virtual bool serializeRequestTemplateMethodPart() override;
    virtual bool parseResponseTemplateMethodPart() override;

};
using PCommandDisconnectSource = std::shared_ptr<CommandDisconnectSource>;

}

#endif // CMD_DISCONNECT_SOURCE_H
