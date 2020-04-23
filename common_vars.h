#ifndef COMMON_VARS_H
#define COMMON_VARS_H

#include <string>

namespace video_server_client{

static const char * PRINT_HEADER = "video-server-client:";

namespace common_vars {

    static const std::string VIDEO_SERVER_DYNATTR_ONLINE = "online";

    namespace incoming_commands {
        const std::string COMMAND_TYPE = "cmd_type";
        const std::string COMMAND_NAME = "cmd_name";

        const std::string COMMAND_EVENT = "event";
        const std::string COMMAND_STATE_CHANGED = "state_changed";

        const std::string COMMAND_RESULT_SUCCESS = "success";
        const std::string COMMAND_RESULT_FAIL = "fail";
    }
}

}

#endif // COMMON_VARS_H
