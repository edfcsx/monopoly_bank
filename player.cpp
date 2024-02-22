#include <iostream>
#include "player.h"

Player::Player(
        string username,
        string password,
        std::shared_ptr<asio::ip::tcp::socket> sock
) :
    m_username(std::move(username)),
    m_password(std::move(password)),
    m_connection(std::make_unique<Connection>(sock)),
    m_money(100'000)
{
    // send the profile to the client
    SendProfile();
}

Player::~Player() {}

void Player::AttachConnection(std::shared_ptr<asio::ip::tcp::socket> sock) {
    if (m_connection && m_connection->IsOpen()) {
        m_connection->Close();

        // wait for the connection to close
        while (m_connection->IsOpen()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    m_connection = std::make_unique<Connection>(sock);
}

string Player::GetPassword() const {
    return m_password;
}

void Player::DispatchMessages() {
    if (m_connection && m_connection->IsOpen())
        m_connection->DispatchMessages();
}

void Player::SendProfile() {
    if (m_connection && m_connection->IsOpen()) {

        nlohmann::json j = {
            {"code", SERVER_CODES::SEND_PROFILE},
            {"username", m_username},
            {"money", m_money}
        };

        m_connection->Send(j);
    }
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
