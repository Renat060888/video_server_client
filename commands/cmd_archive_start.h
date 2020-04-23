#ifndef CMD_ARCHIVE_START_H
#define CMD_ARCHIVE_START_H

#include <string>

#include "i_command.h"
#include "common_types_private.h"

namespace video_server_client{

class CommandArchiveStart : public ICommand {
    // TODO: private CTOR/DTOR
    friend class VideoServerClient;
public:
    struct SInitialParams {
        SInitialParams()
            : sensorId(0)
            , asyncOperations(false)
        {}
        uint64_t sensorId;
        std::string archivingName;
        uint64_t correlationId;
        bool asyncOperations;

        // creating from status
        std::string archivingId;
        EArchiveState archiveState;
    };

    CommandArchiveStart( common_types::SCommandServices * _commandServices );

    bool init( SInitialParams _params );

    // NOTE: response from server
    std::string m_archivingId;
    EArchiveState m_archiveState;


protected:
    virtual bool parseResponse( const std::string & _msgBody ) override;
    virtual std::string execDerive() override;


private:
    virtual bool serializeRequestTemplateMethodPart() override;
    virtual bool parseResponseTemplateMethodPart() override;

};
using PCommandArchiveStart = std::shared_ptr<CommandArchiveStart>;

}

#endif // CMD_ARCHIVE_START_H
