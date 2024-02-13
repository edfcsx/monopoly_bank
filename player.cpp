#include "player.h"

Player::Player(string username, string password) :
    m_username(std::move(username)),
    m_password(std::move(password))
{}

Player::~Player() {}
