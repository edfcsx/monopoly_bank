#ifndef PLAYER_H_
#define PLAYER_H_

#include <string>
#include "connection.h"
#include "networking.h"

using std::string;

class Connection;

class Player
{
public:
    Player(string username, string password, std::shared_ptr<asio::ip::tcp::socket> sock);
    ~Player();

    string m_username;
    string m_password;

    void AttachConnection(std::shared_ptr<asio::ip::tcp::socket> sock);

private:
    std::unique_ptr<Connection> m_connection;
};

#endif // PLAYER_H_