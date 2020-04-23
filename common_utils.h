#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include "from_ms_common/common/ms_common_utils.h"
#include "common_types_private.h"

namespace video_server_client{
namespace common_utils{

// ------------------------------------------------------------------
// converters
// ------------------------------------------------------------------
inline std::string convertAnalyzeStateToStr( const EAnalyzeState _state ){

    switch( _state ){
    case EAnalyzeState::ACTIVE: return "ACTIVE";
    case EAnalyzeState::CRUSHED: return "CRUSHED";
    case EAnalyzeState::PREPARING: return "PREPARING";
    case EAnalyzeState::READY: return "READY";
    case EAnalyzeState::UNAVAILABLE: return "UNAVAILABLE";
    case EAnalyzeState::UNDEFINED: return "UNDEFINED";
    default: {
        std::cout << "analyze state: " << (int)_state << std::endl;
        assert( false && "unknown analyze state" );
    }
    }
}

inline EAnalyzeState convertAnalyzeStateStr( const std::string & _state ){

    if( "ACTIVE" == _state ){
        return EAnalyzeState::ACTIVE;
    }
    else if( "CRUSHED" == _state ){
        return EAnalyzeState::CRUSHED;
    }
    else if( "PREPARING" == _state ){
        return EAnalyzeState::PREPARING;
    }
    else if( "READY" == _state ){
        return EAnalyzeState::READY;
    }
    else if( "UNAVAILABLE" == _state ){
        return EAnalyzeState::UNAVAILABLE;
    }
    else if( "UNDEFINED" == _state ){
        return EAnalyzeState::UNDEFINED;
    }
    else{
        std::cout << "analyze state str: " << _state << std::endl;
        assert( false && "unknown analyze state str" );
    }
}

inline std::string convertArchivingStateToStr( const EArchiveState _state ){

    switch( _state ){
    case EArchiveState::ACTIVE: return "ACTIVE";
    case EArchiveState::CRUSHED: return "CRUSHED";
    case EArchiveState::PREPARING: return "PREPARING";
    case EArchiveState::READY: return "READY";
    case EArchiveState::UNAVAILABLE: return "UNAVAILABLE";
    case EArchiveState::UNDEFINED: return "UNDEFINED";
    default: {
        std::cout << "archive state: " << (int)_state << std::endl;
        assert( false && "unknown archive state" );
    }
    }
}

inline EArchiveState convertArchivingStateStr( const std::string & _state ){

    if( "ACTIVE" == _state ){
        return EArchiveState::ACTIVE;
    }
    else if( "CRUSHED" == _state ){
        return EArchiveState::CRUSHED;
    }
    else if( "PREPARING" == _state ){
        return EArchiveState::PREPARING;
    }
    else if( "READY" == _state ){
        return EArchiveState::READY;
    }
    else if( "UNAVAILABLE" == _state ){
        return EArchiveState::UNAVAILABLE;
    }
    else if( "UNDEFINED" == _state ){
        return EArchiveState::UNDEFINED;
    }
    else{
        std::cout << "archive state str: " << _state << std::endl;
        assert( false && "unknown archive state str" );
    }
}

}
}

#endif // COMMON_UTILS_H
