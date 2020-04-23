
#include "from_ms_common/system/logger.h"
#include "i_command.h"

namespace video_server_client {

using namespace std;

ICommand::ICommand( common_types::SCommandServices * _commandServices, PNetworkClient _network )
    : m_commandServices(_commandServices)
{
    m_networkRequest = _network->getRequestInstance();
}

ICommand::~ICommand(){

}

// async section
bool ICommand::execAsync(){

    if( m_networkRequest->isPerforming() ){
        m_lastError = "request already performed, correlation id [" + m_networkRequest->m_correlationId + "] msg [" + m_outcomingMsg + "]";
        return false;
    }

    if( ! serializeRequestTemplateMethodPart() ){
        return false;
    }

    if( ! performAsyncNetworking() ){
        return false;
    }

    return true;
}

bool ICommand::isReady(){

    if( ! m_networkRequest->isPerforming() ){
        return true;
    }

    if( ! m_networkRequest->checkResponseReadyness() ){
        if( m_networkRequest->isTimeouted() ){
            return true;
        }
        return false;
    }

    m_incomingMsg = m_networkRequest->getAsyncResponse();

    if( m_incomingMsg.find("pong") == std::string::npos ){
        VS_LOG_INFO << "parsed response [" << m_incomingMsg << "]" << endl;
    }

    if( ! parseResponseTemplateMethodPart() ){
        return false;
    }

    return true;
}

bool ICommand::performAsyncNetworking(){

    m_networkRequest->sendMessageAsync( m_outcomingMsg );
    return true;
}

// sync section
bool ICommand::exec(){

    if( ! serializeRequestTemplateMethodPart() ){
        return false;
    }

    if( ! performBlockedNetworking() ){
        return false;
    }

    if( ! parseResponseTemplateMethodPart() ){
        return false;
    }

    return true;
}

bool ICommand::performBlockedNetworking(){

    m_networkRequest->setOutcomingMessage( m_outcomingMsg );
    m_incomingMsg = m_networkRequest->getIncomingMessage();
    return true;
}

}
