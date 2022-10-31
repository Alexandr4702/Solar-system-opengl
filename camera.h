#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <thread>
#include <math.h>
#include <mutex>
#include <functional>

#include <string>

#include "eigen/Eigen/Core"
#include "eigen/Eigen/Geometry"

class Camera
{
    public:
    void setProjetionMatrix(Eigen::Matrix4f mat);
    void setProjetionMatrix(float fovY, float aspectRatio, float zNear, float zFar);
    Eigen::Vector3f getTranslation() const;
    Eigen::Quaternionf getRotation() const;
    Eigen::Vector3f getScale() const;
    void TranslateCam(Eigen::Vector3f transl);
    void setTranslationCam(Eigen::Vector3f transl);
    void rotateCam(Eigen::Quaternionf rot);
    void setCamRotation(Eigen::Quaternionf& rot);
    void ScaleCam(Eigen::Vector3f scal);
    Eigen::Matrix4f getCameraMatrix() const;
    Eigen::Matrix4f getCameraProjectiveMatrix() const;
    void printCameraParam(std::ostream& out) const;
    static Eigen::Matrix4f projective_matrix(float fovY, float aspectRatio, float zNear, float zFar);

    private:
    Eigen::Vector3f translation{0, 0, 1};
    Eigen::Quaternionf rotation = Eigen::Quaternionf(0, 0, 1, 0);
    Eigen::Vector3f       scale {1.0f, 1.0f, 1.0f};
    Eigen::Matrix4f projectiveMatrix = projective_matrix(60.f, 1.f, 1.f, 100.0f);
    mutable std::mutex mtx;
};

class TimeMeasure
{
public:
    TimeMeasure()
    {
        start = std::chrono::system_clock::now();
    }
    TimeMeasure(double* save)
    {
        save_diff = save;
        start = std::chrono::system_clock::now();
    }
    ~TimeMeasure()
    {
        stop = std::chrono::system_clock::now();
        auto diff = stop - start;
        if(save_diff)
            *save_diff = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
        std::cout << "Time in microsec: " << std::chrono::duration_cast<std::chrono::microseconds>(diff).count() << std::endl;
    }
private:
    double* save_diff = nullptr;
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point stop;
};

#endif // __CAMERA_H__