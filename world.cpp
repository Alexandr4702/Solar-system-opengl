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

    while (_isPaintThreadRun.load(std::memory_order_relaxed))
    {
        calcForcesAccelrations();

        for (Body &body : _bodies)
        {
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
    // std::cout << "New iter \n";
    for (uint32_t i = 0; i < _bodies.size(); i++)
    {

        Body::BodyKinematicParametrs param_i(_bodies[i].getBodyKinematicParametrs());

        for (uint32_t j = i + 1; j < _bodies.size(); j++)
        {
            Body::BodyKinematicParametrs param_j(_bodies[j].getBodyKinematicParametrs());

            Vector3d r_i_j = param_i.postition - param_j.postition;
            Vector3d r_i_j_normalised = r_i_j.normalized();
            double r_square = r_i_j.squaredNorm();
            double force_module = G * _bodies[i].getMass() * _bodies[j].getMass() / r_square;

            param_i.force += -r_i_j_normalised * force_module;
            param_j.force += r_i_j_normalised * force_module;
            _bodies[j].setBodyKinematicParametrs(std::move(param_j));

            // std::cout << _bodies[j].getName() << " " << param_j.postition.transpose() << " j\n";
        }
        param_i.acceleration = param_i.force / _bodies[i].getMass();
        param_i.force = Vector3d::Zero();

        // std::cout << _bodies[i].getName() << " " << param_i.postition.transpose() << "\n";

        _bodies[i].setBodyKinematicParametrs(std::move(param_i));
    }
}
