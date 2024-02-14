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
    Connection(std::shared_ptr<asio::ip::tcp::socket> sock, std::shared_ptr<std::unordered_map<std::string, Player *>> players);
    ~Connection();

    void Close();
    [[nodiscard]] bool IsOpen() const;

private:
    bool m_isOpen;
    std::shared_ptr<asio::ip::tcp::socket> m_sock;
    std::shared_ptr<std::unordered_map<std::string, Player *>> m_players;
    asio::streambuf m_request;
    std::string m_response;

    vector<nlohmann::json> m_messagesIn;
    vector<nlohmann::json> m_messagesOut;

    void IOListener();
};

#endif // CONNECTION_H_