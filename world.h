#ifndef WORLD_H
#define WORLD_H
#include <vector>
#include <unordered_map>

#include "body.h"

/*
@brief Class that contains bodies and performs the world evolution calculation.
*/
class World
{
public:
    World();
    std::vector<Body> _bodies;
};

#endif // WORLD_H
