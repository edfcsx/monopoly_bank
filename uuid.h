#ifndef UUID_H_
#define UUID_H_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>

class UUID
{
public:
    static std::string v4 () {
        boost::uuids::random_generator gen;
        boost::uuids::uuid uuid = gen();
        return boost::uuids::to_string(uuid);
    }
};

#endif