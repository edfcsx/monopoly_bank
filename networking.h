#ifndef NETWORKING_H_
#define NETWORKING_H_

#include <boost/predef.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif // !_WIN32_WINNT

#if BOOST_OS_WINDOWS
#if _WIN32_WINNT <= 0x0502 // Windows Server 2003 or earlier
#define BOOST_ASIO_DISABLE_IOCP
#define BOOST_ASIO_ENABLE_CANCELIO
#endif // _WIN32_WINNT <= 0x0502
#endif // BOOST_OS_WINDOWS

#include <boost/asio.hpp>
using namespace boost;

#include <map>

enum SERVER_CODES: unsigned int
{
    UNKNOWN = 0,
    LIMIT_REACHED,
    NEED_AUTHENTICATE,
    AUTHENTICATE_FAILED,
    AUTHENTICATE
};

struct AuthorizationRequest
{
    std::string header;
    std::string username;
    std::string password;
};

#endif // NETWORKING_H_
