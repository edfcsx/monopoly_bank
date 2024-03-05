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

#include <memory>
#include <boost/asio.hpp>
using namespace boost;

#include "json.hpp"

typedef std::shared_ptr<asio::ip::tcp::socket> ptr_socket;

enum SERVER_CODES: unsigned int
{
    UNKNOWN = 1,
    LIMIT_REACHED,
    NEED_AUTHENTICATE,
    AUTHENTICATE_FAILED,
    AUTHENTICATE,
    AUTHENTICATE_SUCCESS,
    PING,
    PING_RESPONSE,
    SEND_PROFILE,
    TRANSFER,
    TRANSFER_SUCCESS,
    TRANSFER_RECEIVED,
    TRANSFER_NO_FUNDS,
    BAD_REQUEST,
};

struct NetworkingMessage {
    SERVER_CODES code;
    nlohmann::json data;
};

enum ConnectionProtocol: int {
    RAW,
    WEBSOCKET
};

#endif // NETWORKING_H_
