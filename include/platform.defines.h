#pragma once

// coping with std::array initialization with Visual 2013
#ifdef   _WIN32
#define STDARR_INIT ( {
#define STDARR_END } )
#else
#define STDARR_INIT { {
#define STDARR_END } }
#endif
