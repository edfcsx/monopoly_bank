#include "player.h"

Player::Player(string username, string password, std::shared_ptr<asio::ip::tcp::socket> sock) :
    m_username(std::move(username)),
    m_password(std::move(password)),
    m_connection(new Connection(sock))
{}

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
