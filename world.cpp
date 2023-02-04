#include "world.h"

World::World()
{
}

World::~World()
{
    _isPaintThreadRun.store(false, std::memory_order_relaxed);
    _calculatingThread.join();
}

void World::StartSimulations()
{
    _calculatingThread = std::thread(&World::calculatingThread, this);
}

void World::calculatingThread()
{
    using namespace std::literals::chrono_literals;
    using namespace std;

    while(_isPaintThreadRun.load(std::memory_order_relaxed)) {
        for(Body& body: _bodies) {
            Body::BodyKinematicParametrs param(body.getBodyKinematicParametrs());

            param.velocity += param.acceleration * dt;
            param.postition += param.velocity * dt;

            body.setBodyKinematicParametrs(param);
        }
    }
}

void World::calcForcesAccelrations()
{

}



