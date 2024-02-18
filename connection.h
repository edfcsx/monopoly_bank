#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <vector>
#include <string>
#include <unordered_map>
#include "json.hpp"
#include "networking.h"
#include "player.h"

using std::vector;
class Player;

class Connection
{
public:
    Connection(
            std::shared_ptr<asio::ip::tcp::socket> sock);
    ~Connection();

    void Close();
    [[nodiscard]] bool IsOpen() const;
private:
    bool m_isOpen;
    std::shared_ptr<asio::ip::tcp::socket> m_sock;
    asio::streambuf m_request;
    std::string m_response;

    vector<nlohmann::json> m_messagesIn;
    vector<nlohmann::json> m_messagesOut;
public:
    void DispatchMessages();
    void Send(nlohmann::json message);
private:
    void ListenIncomingMessages();
};

#endif // CONNECTION_H_
