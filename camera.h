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
    void setProjetionMatrix(Eigen::Matrix4f mat)
    {
        mtx.lock();
        projectiveMatrix = mat;
    }

    void setProjetionMatrix(float fovY, float aspectRatio, float zNear, float zFar)
    {
        setProjetionMatrix(projective_matrix(fovY, aspectRatio, zNear, zFar));
    }

    Eigen::Vector3f getTranslation() const
    {
        mtx.lock();
        Eigen::Vector3f ret = translation;
        mtx.unlock();
        return ret;
    }
    Eigen::Quaternionf getRotation() const
    {
        mtx.lock();
        Eigen::Quaternionf ret = rotation;
        mtx.unlock();
        return ret;
    }
    Eigen::Vector3f getScale() const
    {
        mtx.lock();
        Eigen::Vector3f ret = scale;
        mtx.unlock();
        return ret;
    }
    void TranslateCam(Eigen::Vector3f transl)
    {
        std::scoped_lock guard(mtx);
        translation += rotation.inverse() * transl;
    }
    void rotateCam(Eigen::Quaternionf rot)
    {
        std::scoped_lock guard(mtx);
        rotation = rot * rotation;
    }
    void setCamRotation(Eigen::Quaternionf& rot)
    {
        if(rot.coeffs().hasNaN())
            return;
        std::scoped_lock guard(mtx);
        rotation = rot;
    }
    void ScaleCam(Eigen::Vector3f scal)
    {
        std::scoped_lock guard(mtx);
        scale += scal;
    }
    Eigen::Matrix4f getCameraMatrix() const
    {
        std::scoped_lock guard(mtx);
        Eigen::Affine3f CamMatrix;
        CamMatrix.setIdentity();
        CamMatrix.scale(scale);
        CamMatrix.rotate(rotation);
        CamMatrix.translate(translation);

        return CamMatrix.matrix();
    }

    Eigen::Matrix4f getCameraProjectiveMatrix() const
    {
        return projectiveMatrix * getCameraMatrix();
    }

    void printCameraParam(std::ostream& out) const
    {
        out << translation.transpose() << "\r\n";
        out << rotation << "\r\n";
    }

    static Eigen::Matrix4f projective_matrix(float fovY, float aspectRatio, float zNear, float zFar)
    {
        float yScale = 1 / tan(fovY * M_PI / 360.0f);
        float xScale = yScale / aspectRatio;

        // float yScale = 1;
        // float xScale = 1;

        Eigen::Matrix4f pmat;
        pmat << xScale, 0, 0, 0,
                0, yScale, 0, 0,
                0, 0, -(zFar+zNear)/(zFar-zNear), -2*zNear*zFar/(zFar-zNear),
                0, 0, -1, 0;
        return pmat;
    }
    private:
    Eigen::Vector3f translation{0, 0, 1};
    Eigen::Quaternionf rotation = Eigen::Quaternionf(1, 0, 0, 0);
    Eigen::Vector3f       scale {1.0f, 1.0f, 1.0f};
    Eigen::Matrix4f projectiveMatrix = projective_matrix(60.f, 1.f, 01.f, 100.0f);
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