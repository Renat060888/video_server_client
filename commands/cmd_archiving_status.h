#ifndef CMD_ARCHIVING_STATUS_H
#define CMD_ARCHIVING_STATUS_H

#include "i_command.h"
#include "common_types_private.h"

namespace video_server_client{

class CommandArchivingStatus : public ICommand
{
public:
    static constexpr uint64_t ALL_SENSORS = 0;
    static const TArchivingId ALL_ARCHIVINGS;

    struct SInitialParams {
        SInitialParams()
        {}

        TArchivingId archivingId;
    };

    CommandArchivingStatus( common_types::SCommandServices * _commandServices );

    // request
    SInitialParams m_params;

    // response
    SArchiveStatus m_status;


private:
    virtual bool serializeRequestTemplateMethodPart() override;
    virtual bool parseResponseTemplateMethodPart() override;
};
using PCommandArchivingStatus = std::shared_ptr<CommandArchivingStatus>;

}

#endif // CMD_ARCHIVING_STATUS_H
