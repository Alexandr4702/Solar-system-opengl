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
    using namespace Eigen;

    while(_isPaintThreadRun.load(std::memory_order_relaxed)) {
        for(Body& body: _bodies) {
            Body::BodyKinematicParametrs param(body.getBodyKinematicParametrs());

            param.velocity += param.acceleration * dt;
            param.postition += param.velocity * dt;

            param.angularVelocity += param.angularAcceleration * dt;

            Quaterniond N;
            N.vec() = param.angularVelocity * dt / 2;
            double omega2 = param.angularVelocity.transpose() * param.angularVelocity;
            double seondOrder = omega2 * dt * dt / 8.0;
            N.w() = 1 - seondOrder;

            param.orientation = param.orientation * N;
            param.orientation.normalize();

            body.setBodyKinematicParametrs(param);
        }
        std::this_thread::sleep_for(15ms);
    }
}

void World::calcForcesAccelrations()
{
    using namespace Eigen;
    for(uint32_t i = 0; i < _bodies.size() - 1; i++) {

        Body::BodyKinematicParametrs param_i(_bodies[i].getBodyKinematicParametrs());
        param_i.acceleration = Vector3d::Zero();

        for(uint32_t j = i + 1; j < _bodies.size(); j++) {
            Body::BodyKinematicParametrs param_j(_bodies[j].getBodyKinematicParametrs());

            Vector3d r_1_2 = param_j.postition - param_i.postition;
            double r_square = r_1_2.squaredNorm();
            double force_module = _bodies[i].getMass() * _bodies[j].getMass() / r_square;

        }
    }
}



