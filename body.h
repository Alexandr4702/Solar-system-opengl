#ifndef BODY_H
#define BODY_H

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QOpenGLTexture>
#include <QImage>

#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <cstddef>
#include <filesystem>

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
     Body(QOpenGLContext*, std::string filename);
     Body(Body&&);
     Body(const Body&) = delete;//TODO
     ~Body();
     void draw(QOpenGLShaderProgram& program);
     void draw(QOpenGLShaderProgram& program, Eigen::Matrix4f& matrixCam);
     void update();

     void setBodyPosition(Eigen::Vector3f&);
     void translateBody(Eigen::Vector3f&);

     void setBodyRotation(Eigen::Quaternionf& q);
     void rotateBody(Eigen::Quaternionf& q);

     Eigen::Matrix4f getBodyMatrix() const;
private:
     bool ImportModel(std::string pFile);

struct VertexData
{
Eigen::Vector3f position;
Eigen::Vector2f texCoord;
Eigen::Vector3f normal;
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

     std::vector<VertexData> vertices;
     std::vector<uint32_t> indices;
     uint32_t numberOfFaces = 0;

     std::shared_ptr<QOpenGLBuffer> arrayBuf;
     std::shared_ptr<QOpenGLBuffer> indexBuf;

     std::shared_ptr<QOpenGLTexture> texture = nullptr;
};

#endif // BODY_H
