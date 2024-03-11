#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <vector>
#include <string>
#include <unordered_map>
#include "json.hpp"
#include "networking.h"

using std::vector;

const unsigned int MSG_MASK_SIZE = 4;
const std::size_t MAX_MESSAGE_SIZE = std::numeric_limits<std::size_t>::max();

namespace connection {
    enum opcode: unsigned char {
        CONTINUATION = 0x0,
        TEXT = 0x1,
        BINARY = 0x2,
        CLOSE = 0x8,
        PING = 0x9,
        PONG = 0xA,
        RAW = 0x99
    };
};

namespace connection {
    struct message {
        unsigned char type;
        std::size_t length;
        asio::streambuf buffer;
        bool unique;
        std::size_t additional_bytes;

        void clear_buffer() {
            buffer.consume(buffer.size());
        }

        void reset() {
            length = 0;
            clear_buffer();
            type = connection::opcode::TEXT;
            unique = false;
        }
    };
};

class Connection {
public:
    Connection(tcp_socket socket, ConnProtocol p);
    ~Connection();

    void Close();
    [[nodiscard]] bool is_open() const;
private:
    bool m_isOpen;
    ConnProtocol m_protocol;
    tcp_socket m_sock;
    asio::streambuf m_request;
    std::string m_response;
    connection::message m_message;

    vector<nlohmann::json> m_messagesOut;
public:
    void DispatchMessages();
    void Send(nlohmann::json message);
private:
    void listen_raw_messages();
    void listen_websocket_messages();
    void read_websocket_message_content();
    void send_data(const std::string & data, connection::opcode c);
};

#endif // CONNECTION_H_
