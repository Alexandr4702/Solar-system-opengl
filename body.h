#ifndef BODY_H
#define BODY_H

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>

#include <vector>
#include <string>
#include <memory>
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
class Body: public QOpenGLFunctions
{
public:
     Body(QOpenGLContext*);
     Body(Body&&);
     Body(const Body&) = delete;
     ~Body();
     void draw(QOpenGLShaderProgram& program, Camera& cam, QOpenGLFunctions* f);
     void update();
     Eigen::Matrix4f getBodyMatrix() const;
private:
     bool ImportModel(std::string pFile);

struct VertexData
{
QVector3D position;
QVector2D texCoord;
};
     bool ImportTestModel();
private:
     QOpenGLContext* ctx;
/*
@brief test func
*/
     void drawCube();

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

     std::unique_ptr<QOpenGLBuffer> arrayBuf;
     std::unique_ptr<QOpenGLBuffer> indexBuf;
};

#endif // BODY_H
