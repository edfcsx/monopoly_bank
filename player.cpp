#include "player.h"

Player::Player(
        string username,
        string password,
        std::shared_ptr<asio::ip::tcp::socket> sock,
        std::shared_ptr<std::unordered_map<string, Player *>> players
) :
    m_username(std::move(username)),
    m_password(std::move(password)),
    m_connection(new Connection(sock, players)),
    m_money(2'558'000)
{}

Player::~Player() {}

void Player::AttachConnection(std::shared_ptr<asio::ip::tcp::socket> sock, std::shared_ptr<std::unordered_map<string, Player *>> players) {
    if (m_connection && m_connection->IsOpen()) {
        m_connection->Close();

        // wait for the connection to close
        while (m_connection->IsOpen()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    m_connection = std::make_unique<Connection>(sock, players);
}

string Player::GetPassword() const {
    return m_password;
}
