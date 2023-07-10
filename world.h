#ifndef WORLD_H
#define WORLD_H
#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <chrono>
#include <functional>

#include <eigen/Eigen/Core>
#include <eigen/Eigen/Geometry>
#include <thread>
#include "body.h"

/*
@brief Class that contains bodies and performs the world evolution calculation.
*/
class World
{
public:
    World();
    ~World();
    void StartSimulations();
    std::vector<Body> _bodies;
    void calculatingThread();

private:
    void calcForcesAccelrations();
    std::thread _calculatingThread;
    std::atomic<bool> _isPaintThreadRun = true;
    double dt = 1e-3;
    const double G = 1e1;
};

template <typename T, typename U>
T &&rungeKutta4(T &vec, U dt)
{
}

#endif // WORLD_H
