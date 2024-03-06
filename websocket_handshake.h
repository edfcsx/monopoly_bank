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
typedef void (*HandshakeCallback)(ptr_socket sock, ConnectionManager * manager);

class WebSocketHandshake
{
public:
    WebSocketHandshake(std::shared_ptr<asio::ip::tcp::socket> sock, ConnectionManager * manager, HandshakeCallback callback);
    ~WebSocketHandshake() = default;

private:
    std::map<std::string, std::string> m_headers;
    std::shared_ptr<asio::ip::tcp::socket> m_sock;
    ConnectionManager * m_manager;
    asio::streambuf m_request_buf;
    HandshakeCallback m_callback;
private:
    void OnHeadersReceived(const boost::system::error_code & ec, std::size_t bytes_transferred);
    void OnFinish(const boost::system::error_code & ec);
    void RespondToHandshake();
    std::string CalculateWebsocketAcceptValue(const std::string & request_key);
};

#endif // WEB_SOCKET_HANDSHAKE_H_