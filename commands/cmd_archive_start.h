#ifndef CMD_ARCHIVE_START_H
#define CMD_ARCHIVE_START_H

#include <string>

#include "i_command.h"
#include "../common_types_private.h"

namespace video_server_client{

class CommandArchiveStart : public ICommand {
    // TODO: private CTOR/DTOR
    friend class VideoServerClient;
public:
    struct SInitialParams {
        SInitialParams()
            : sensorId(0)
            , archiveState(EArchiveState::UNDEFINED)
        {}
        uint64_t sensorId;
        std::string archivingName;
        // creating from status
        std::string archivingId;
        EArchiveState archiveState;
    };

    CommandArchiveStart( common_types::SCommandServices * _commandServices );

    SInitialParams m_params;    
    class ArchiveHandler * m_commandInitiator;


private:
    virtual bool serializeRequestTemplateMethodPart() override;
    virtual bool parseResponseTemplateMethodPart() override;
};
using PCommandArchiveStart = std::shared_ptr<CommandArchiveStart>;

}

#endif // CMD_ARCHIVE_START_H
