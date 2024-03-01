#ifndef AUTHENTICATE_COMMAND_H_
#define AUTHENTICATE_COMMAND_H_

#include <string>
#include "player.h"
#include "icommand.h"
#include "server.h"

class AuthenticateCommand : public Icommand
{
public:
    void execute(std::shared_ptr<std::unordered_map<std::string, Player *>> m_players, nlohmann::json data) override {

    };
};

#endif // AUTHENTICATE_COMMAND_H_