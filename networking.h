#ifndef _ASIO_COMMON_H_
#define _ASIO_COMMON_H_

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

enum REJECT_REASON: unsigned int
{
    REJECT_REASON_UNKNOWN = 0,
    REJECT_REASON_LIMIT_REACHED
};

std::map<std::string, std::string> message_headers = {
    { "REJECT_CONNECTION_LIMIT_REACHED", "REJECT_CONNECTION_LIMIT_REACHED" },
    { "REJECT_CONNECTION_UNKNOWN", "REJECT_CONNECTION_UNKNOWN" },
    { "AUTHENTICATE", "AUTHENTICATE" }
};

struct AuthorizationRequest
{
    std::string header;
    std::string username;
    std::string password;
};

#endif // !_ASIO_COMMON_H_
