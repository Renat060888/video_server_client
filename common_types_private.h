#ifndef COMMON_TYPES_PRIVATE_H
#define COMMON_TYPES_PRIVATE_H

#include "from_ms_common/communication/network_interface.h"
#include "common_types.h"

namespace video_server_client {
namespace common_types {


// typedefs
using TLaunchCorrelationId = std::string;

// interfaces
class ICommandCallbacksObserver {
public:
    virtual ~ICommandCallbacksObserver(){}

    virtual void pongCatched() = 0;
    virtual void archiverIsReady( const TArchivingId & _archId, const TLaunchCorrelationId & _corrId ) = 0;
    virtual void analyzerIsReady( const TProcessingId & _procId, const TLaunchCorrelationId & _corrId ) = 0;
    virtual void updateServerState( const SServerState & _state ) = 0;
    virtual void updateSystemState( const SSystemState & _state ) = 0;
    virtual void updateArchivingStatus( const std::vector<SArchiveStatus> & _status ) = 0;
    virtual void updateAnalyzeStatus( const std::vector<SAnalyzeStatus> & _status ) = 0;
    virtual void newEvent( std::vector<SAnalyticEvent> && _event ) = 0;
};

// containers
struct SCommandServices {
    // make requests
    PNetworkClient networkClient;
    // reflect responses
    ICommandCallbacksObserver * callbacks;
};


}
}

#endif // COMMON_TYPES_PRIVATE_H
