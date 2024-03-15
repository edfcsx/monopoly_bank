#ifndef PLAYER_H_
#define PLAYER_H_

#include <string>
#include "networking.h"

using std::string;

class Player
{
public:
    Player(string username, string password);
    ~Player();
private:
    string m_username;
    string m_password;
    uint   m_money;
public:
    [[nodiscard]] string GetPassword() const;

    void SetMoney(uint money);
    uint GetMoney();
    bool Withdraw(uint amount);
    void IncreaseMoney(uint amount);
};

#endif // PLAYER_H_