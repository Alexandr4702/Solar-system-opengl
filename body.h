#ifndef BODY_H
#define BODY_H

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QGLContext>

#include <vector>
#include <string>

#include <mutex>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <eigen/Eigen/Core>
#include <eigen/Eigen/Geometry>

#include "camera.h"

/*
@brief Class contains information about body and his 3d model.
*/
class Body
{
public:
     Body();
     Body(Body&&);
     Body(const Body&);
     ~Body();
     void draw(QOpenGLShaderProgram& program, Camera& cam);
     void update();
     Eigen::Matrix4f getBodyMatrix() const;
private:
     bool ImportModel(std::string pFile);
private:
     Eigen::Matrix3f J;
     double mass;

     Eigen::Vector3f scale = {1, 1, 1};

     Eigen::Vector3f postition = {0,0,0};
     Eigen::Vector3f velocity = {0,0,0};
     Eigen::Vector3f acceleration = {0,0,0};

     Eigen::Quaternionf orientation = Eigen::Quaternionf::Identity();
     Eigen::Vector3f angularVelocity = {0,0,0};
     Eigen::Vector3f angularAcceleration = {0,0,0};
     mutable std::mutex mtx;

     std::vector<Eigen::Vector3f> vertices;
     std::vector<uint32_t> indices;
     uint32_t numberOfFaces = 0;

     QOpenGLBuffer arrayBuf;
     QOpenGLBuffer indexBuf;
};

#endif // BODY_H
