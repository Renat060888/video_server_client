#ifndef CMD_PING_H
#define CMD_PING_H

#include "i_command.h"
#include "common_types_private.h"

namespace video_server_client {

class CommandPlayerPing : public ICommand
{
public:
    CommandPlayerPing( common_types::SCommandServices * _commandServices, PNetworkClient _network );

    common_types::TPlayerClientUniqueId m_userIdToPlayer;


private:
    virtual bool serializeRequestTemplateMethodPart() override;
    virtual bool parseResponseTemplateMethodPart() override;
};
using PCommandPlayerPing = std::shared_ptr<CommandPlayerPing>;

}

#endif // CMD_PING_H
