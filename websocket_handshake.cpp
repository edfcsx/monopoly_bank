#include "websocket_handshake.h"

WebSocketHandshake::WebSocketHandshake(tcp_socket sock, handshake_callback c):
    m_sock(sock),
    m_callback(c)
{
    // read handshake request status line
    asio::async_read_until(*m_sock, m_request_buf, "\r\n\r\n",
        [this](const system::error_code & ec, std::size_t bytes_transferred) {
            on_headers_received(ec, bytes_transferred);
        }
    );
}

void WebSocketHandshake::on_finish(const boost::system::error_code & ec)
{
    if (ec.value() != 0) {
        std::cout << "[Server] failed to read websocket request: " << ec.message() << std::endl;
        m_sock->close();
    } else {
        m_callback(m_sock);
    }

    delete this;
}

void WebSocketHandshake::on_headers_received(const boost::system::error_code & ec, std::size_t bytes_transfered)
{
    if (ec.value() != 0) {
        on_finish(ec);
        return;
    }

    std::string request_data;
    std::istream is(&m_request_buf);
    std::getline(is, request_data);

    // check protocol is HTTP/1.1
    if (request_data.find("HTTP/1.1") == std::string::npos) {
        on_finish({boost::system::errc::make_error_code(boost::system::errc::protocol_error)});
        return;
    }

    // parse headers
    std::string header_name, header_value;

    while (true) {
        std::getline(is, request_data);

        if (request_data.empty()) break;

        auto separator_pos = request_data.find(':');

        if (separator_pos != std::string::npos) {
            header_name = request_data.substr(0, separator_pos);

            if (separator_pos < request_data.length() - 1) {
                header_value = request_data.substr(separator_pos + 1);
                header_value.erase(0, header_value.find_first_not_of(' '));

                if (header_value.back() == '\r')
                    header_value.pop_back();
            }
            else
                header_value = "";

            m_headers[header_name] = header_value;
        }
    }

    if (m_headers.find("Sec-WebSocket-Key") == m_headers.end()) {
        on_finish({boost::system::errc::make_error_code(boost::system::errc::protocol_error)});
        return;
    } else {
        response_request();
    }
}

void WebSocketHandshake::response_request()
{
    std::string acceptValue = generate_handshake_accept_value(m_headers["Sec-WebSocket-Key"]);

    auto * response = new std::string{
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: " + acceptValue +
        "\r\n\r\n"
    };

    asio::async_write(*m_sock, asio::buffer(*response),
        [this, response](const system::error_code & ec, std::size_t bytes_transferred) {
            on_finish(ec);
            delete response;
        }
    );
}

std::string WebSocketHandshake::generate_handshake_accept_value(const std::string &request_key) {
    std::string magic_string = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string combined = request_key + magic_string;

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(combined.data()), combined.size(), hash);

    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, hash, SHA_DIGEST_LENGTH);
    BIO_flush(b64);

    BUF_MEM* bptr;
    BIO_get_mem_ptr(b64, &bptr);

    std::string output(bptr->data, bptr->length - 1); // -1 to remove newline at the end

    BIO_free_all(b64);

    return output;
}