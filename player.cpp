#include <iostream>
#include <utility>
#include "player.h"

Player::Player(string username,string password) :
    m_username(std::move(username)),
    m_password(std::move(password)),
    m_money(100'000)
{}

Player::~Player() {}

string Player::GetPassword() const {
    return m_password;
}

void Player::SetMoney(uint money) {
    m_money = money;
}

uint Player::GetMoney() {
    return m_money;
}

bool Player::Withdraw(uint amount) {
    if (m_money >= amount) {
        m_money -= amount;
        return true;
    }

    return false;
}

void Player::IncreaseMoney(uint amount) {
    m_money += amount;
}
