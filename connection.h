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

    enum status: uint16_t {
        NORMAL_CLOSURE = 1000,
        GOING_AWAY = 1001,
        PROTOCOL_ERROR = 1002,
        UNSUPPORTED_DATA = 1003,
        NO_STATUS_RCVD = 1005,
        ABNORMAL_CLOSURE = 1006,
        INVALID_FRAME_PAYLOAD_DATA = 1007,
        POLICY_VIOLATION = 1008,
        MESSAGE_TOO_BIG = 1009,
        MANDATORY_EXT = 1010,
        DEF_STATUS = 9999
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

    using success_send_callback = std::function<void()>;
    using on_close_callback = std::function<void(std::string ip, bool playing)>;
};

class Connection {
public:
    Connection(tcp_socket socket, ConnProtocol p, connection::on_close_callback on_close = nullptr);
    ~Connection();
private:
    std::string m_ip;
    bool m_isOpen;
    ConnProtocol m_protocol;
    tcp_socket m_sock;
    connection::message m_message;
    connection::on_close_callback m_close_callback;
    bool m_playing = false;
    vector<unsigned char> m_send_frame;
    vector<nlohmann::json> m_messagesOut;
public:
    void DispatchMessages();
    void Send(nlohmann::json message);
    void close_connection();
    [[nodiscard]] bool is_open() const;
private:
    void listen_raw_messages();
    void listen_websocket_messages();
    void read_websocket_message_content();
    void send_data(const std::string & data);

    void send_data(
        const std::string & data,
        connection::opcode c,
        connection::status s = connection::status::DEF_STATUS,
        connection::success_send_callback on_success = nullptr);

    void close_websocket(
        connection::status s = connection::status::NORMAL_CLOSURE,
        const std::string & reason = "");

    void set_playing(bool p);
};

#endif // CONNECTION_H_
