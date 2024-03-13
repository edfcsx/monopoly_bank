#ifndef ICOMMAND_H_
#define ICOMMAND_H_

#include <memory>
#include <string>
#include "networking.h"
#include "json.hpp"
#include "player.h"

class Icommand {
public:
    virtual ~Icommand() = default;
    virtual void execute(nlohmann::json data) = 0;
};

#endif // ICOMMAND_H_
