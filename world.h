#ifndef WORLD_H
#define WORLD_H
#include <vector>
#include "body.h"

class World
{
public:
    World();
    std::vector<Body> bodies;
};

#endif // WORLD_H
