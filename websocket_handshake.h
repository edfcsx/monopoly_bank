#ifndef WEB_SOCKET_HANDSHAKE_H_
#define WEB_SOCKET_HANDSHAKE_H_

#include <map>
#include <string>
#include <memory>
#include <iostream>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include "networking.h"

class ConnectionManager;

using handshake_callback = std::function<void(ptr_socket sock)>;
class WebSocketHandshake
{
public:
    WebSocketHandshake(tcp_socket socket, handshake_callback c);
    ~WebSocketHandshake() = default;

private:
    std::map<std::string, std::string> m_headers;
    std::shared_ptr<asio::ip::tcp::socket> m_sock;
    asio::streambuf m_request_buf;
    handshake_callback m_callback;
private:
    void on_headers_received(const boost::system::error_code & ec, std::size_t bytes_transferred);
    void on_finish(const boost::system::error_code & ec);
    void response_request();
    std::string generate_handshake_accept_value(const std::string & request_key);
};

#endif // WEB_SOCKET_HANDSHAKE_H_