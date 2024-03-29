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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera
{
public:
    struct CamParametrs
    {
        float near_plane = .1f;
        float far_plane = 1000.0f;
        float aspect_ratio = 1;
        float fov = 60.f;
    };

    Camera();
    void setAspectRatio(float ratio);
    void setProjetionMatrix(Eigen::Matrix4d mat);
    void setProjetionMatrix(float fovY, float aspectRatio, float zNear, float zFar);
    Eigen::Matrix4d getProjetionMatrix() const;
    Eigen::Vector3d getTranslation() const;
    Eigen::Quaterniond getRotation() const;
    Eigen::Vector3d getScale() const;
    void TranslateCam(Eigen::Vector3d &transl);
    void TranslateCam(Eigen::Vector3d &&transl);
    void setTranslationCam(Eigen::Vector3d &transl);
    void setTranslationCam(Eigen::Vector3d &&transl);
    void rotateCam(Eigen::Quaterniond &rot);
    void rotateCam(Eigen::Quaterniond &&rot);
    void rotateCam(Eigen::Vector2d &mouseCoord);
    void setCamRotation(Eigen::Quaterniond &rot);
    void setCamRotation(Eigen::Quaterniond &&rot);
    void ScaleCam(Eigen::Vector3d &scal);
    Eigen::Matrix4d getCameraMatrix() const;
    Eigen::Matrix4f getCameraProjectiveMatrix() const;
    void printCameraParam(std::ostream &out) const;
    void StartRotation(Eigen::Vector2d &mouseCoord);
    const CamParametrs &getCamParametrs() const
    {
        return m_cam_params;
    }

    static Eigen::Matrix4d projective_matrix(const CamParametrs &param);
    static Eigen::Matrix4d projective_matrix(float fovY, float aspectRatio, float zNear, float zFar);

private:
    CamParametrs m_cam_params;

    Eigen::Vector3d translation{0, 0, 0};
    Eigen::Quaterniond rotation = Eigen::Quaterniond(1, 0, 0, 0);
    Eigen::Vector3d scale{1.0f, 1.0f, 1.0f};
    Eigen::Matrix4d projectiveMatrix = projective_matrix(m_cam_params);

    Eigen::Quaterniond q0;
    Eigen::Vector2d mouseCoord0;

    mutable std::mutex mtx;
};

class TimeMeasure
{
public:
    TimeMeasure()
    {
        start = std::chrono::system_clock::now();
    }
    TimeMeasure(double *save)
    {
        save_diff = save;
        start = std::chrono::system_clock::now();
    }
    ~TimeMeasure()
    {
        stop = std::chrono::system_clock::now();
        auto diff = stop - start;
        if (save_diff)
            *save_diff = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
        std::cout << "Time in microsec: " << std::chrono::duration_cast<std::chrono::microseconds>(diff).count() << std::endl;
    }

private:
    double *save_diff = nullptr;
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point stop;
};

#endif // __CAMERA_H__