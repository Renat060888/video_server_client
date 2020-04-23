#ifndef CMD_ARCHIVE_STOP_H
#define CMD_ARCHIVE_STOP_H

#include "i_command.h"
#include "common_types_private.h"

namespace video_server_client{

class CommandArchiveStop : public ICommand
{
    // TODO: private CTOR/DTOR
    friend class VideoServerClient;
public:
    struct SInitialParams {
        SInitialParams()
            : destroy(false)
        {}
        TArchivingId archivingId;
        bool destroy;
    };

    CommandArchiveStop( common_types::SCommandServices * _commandServices );

    // request
    SInitialParams m_params;


private:
    virtual bool serializeRequestTemplateMethodPart() override;
    virtual bool parseResponseTemplateMethodPart() override;
};
using PCommandArchiveStop = std::shared_ptr<CommandArchiveStop>;

}

#endif // CMD_ARCHIVE_STOP_H
