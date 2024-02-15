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
    Player(
            string username,
            string password,
            std::shared_ptr<asio::ip::tcp::socket> sock,
            std::shared_ptr<std::unordered_map<string, Player *>> players
          );

    ~Player();

private:
    std::unique_ptr<Connection> m_connection;

    string m_username;
    string m_password;
    uint   m_money;

public:
    void AttachConnection(
            std::shared_ptr<asio::ip::tcp::socket> sock,
            std::shared_ptr<std::unordered_map<string, Player *>> players);

    [[nodiscard]] string GetPassword() const;

    void DispatchMessages();
};

#endif // PLAYER_H_