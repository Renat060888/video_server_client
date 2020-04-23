#ifndef I_COMMAND_H
#define I_COMMAND_H

#include <string>

#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/writer.h>

#include "common_types_private.h"

namespace video_server_client {

class ICommand
{
public:
    ICommand( common_types::SCommandServices * _commandServices );
    virtual ~ICommand();

    const std::string & getLastError(){ return m_lastError; }

    bool exec();

    bool execAsync();
    bool isReady();


protected:
    virtual bool serializeRequestTemplateMethodPart() = 0;
    virtual bool parseResponseTemplateMethodPart() = 0;

    // data
    std::string m_outcomingMsg;
    std::string m_incomingMsg;
    std::string m_lastError;

    // service
    common_types::SCommandServices * m_commandServices;
    Json::FastWriter m_jsonWriter;
    Json::Reader m_jsonReader;


private:
    bool performBlockedNetworking();
    bool performAsyncNetworking();

    // data

    // service
    PEnvironmentRequest m_networkRequest;
};
using PCommand = std::shared_ptr<ICommand>;

}

#endif // I_COMMAND_H
