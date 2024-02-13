#ifndef PLAYER_H_
#define PLAYER_H_

#include <string>

using std::string;

class Player
{
public:
    Player(string username, string password);
    ~Player();

    string m_username;
    string m_password;
};

#endif // PLAYER_H_