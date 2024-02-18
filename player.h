#ifndef PLAYER_H_
#define PLAYER_H_

#include <string>
#include "connection.h"
#include "networking.h"
#include "types.h"

using std::string;

class Connection;

class Player
{
public:
    Player(string username, string password, std::shared_ptr<asio::ip::tcp::socket> sock);
    ~Player();

    std::unique_ptr<Connection> m_connection;
private:
    string m_username;
    string m_password;
    uint   m_money;
public:
    void AttachConnection(std::shared_ptr<asio::ip::tcp::socket> sock);
    [[nodiscard]] string GetPassword() const;

    void DispatchMessages();
    void SendProfile();
};

#endif // PLAYER_H_