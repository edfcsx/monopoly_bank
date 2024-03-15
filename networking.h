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

// types from asio
typedef std::shared_ptr<asio::ip::tcp::socket> ptr_socket;
typedef std::shared_ptr<asio::ip::tcp::socket> tcp_socket;
typedef asio::ip::tcp::endpoint tcp_endpoint;
typedef asio::ip::tcp::acceptor tcp_acceptor;

// types from basic types
typedef unsigned int uint;

namespace server {
    enum actions : unsigned int {
        unknown = 1,
        need_authenticate,
        authenticate_failed,
        authenticate,
        authenticate_success,
        ping,
        pong,
        send_profile,
        transfer,
        transfer_success,
        transfer_received,
        transfer_no_funds,
        bad_request,
    };

    enum protocol : unsigned int {
        raw,
        websocket
    };
}

#endif // NETWORKING_H_

