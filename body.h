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

struct VertexData
{
     Eigen::Vector3f position;
     Eigen::Vector2f texCoord;
     Eigen::Vector3f normal;
};

class Mesh : public QOpenGLFunctions
{
     public:
     Mesh(QOpenGLContext* ctx_) : QOpenGLFunctions(ctx_), ctx(ctx_),
     indexBuf( new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer )),
     arrayBuf( new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer))
     {
          arrayBuf->create();
          indexBuf->create();
     }

     Mesh(Mesh&& oth): QOpenGLFunctions(oth.ctx)
     {
          vertices = move(oth.vertices);
          indices  = move(oth.indices);
          raw_texture = std::move(oth.raw_texture);

          arrayBuf = move(oth.arrayBuf);
          indexBuf = move(oth.indexBuf);
          texture = move(oth.texture);

          ctx = oth.ctx;
          oth.ctx = nullptr;
     }

     Mesh(const Mesh& oth): QOpenGLFunctions(oth.ctx),
     indexBuf( new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer )),
     arrayBuf( new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer))
     {
          vertices = oth.vertices;
          indices  = oth.indices;
          raw_texture = oth.raw_texture;

          arrayBuf->create();
          indexBuf->create();

          arrayBuf->bind();
          arrayBuf->allocate(vertices.data(), vertices.size() * sizeof(vertices[0]));

          indexBuf->bind();
          indexBuf->allocate(indices.data(), indices.size() * sizeof(indices[0]));

          texture = std::shared_ptr<QOpenGLTexture> (new QOpenGLTexture(raw_texture.mirrored()));

          ctx = oth.ctx;
     }

     // Mesh(const Mesh& oth) = delete;

     ~Mesh()
     {
          if(arrayBuf.get() != nullptr)
          {
               arrayBuf->destroy();
          }
          if(indexBuf.get() != nullptr)
          {
               indexBuf->destroy();
          }
          if(texture.get() != nullptr)
          {
               texture->destroy();
          }
     }
     void draw(QOpenGLShaderProgram &program);

     std::vector<VertexData> vertices;
     std::vector<uint32_t> indices;

     QImage raw_texture;

     std::shared_ptr<QOpenGLBuffer> arrayBuf;
     std::shared_ptr<QOpenGLBuffer> indexBuf;

     std::shared_ptr<QOpenGLTexture> texture = nullptr;

     private:
     QOpenGLContext *ctx;
};

/*
@brief Class contains information about body and his 3d model.
*/
class Body
{
public:
     Body(QOpenGLContext *);
     Body(QOpenGLContext *, std::string filename);
     Body(Body &&);
     Body(const Body &) = delete; // TODO
     ~Body();
     void draw(QOpenGLShaderProgram &program);
     void draw(QOpenGLShaderProgram &program, Eigen::Matrix4f &matrixCam);
     void update();

     void setBodyPosition(Eigen::Vector3f &);
     void setBodyPosition(Eigen::Vector3f &&);
     void translateBody(Eigen::Vector3f &);

     void setBodyRotation(Eigen::Quaternionf &q);
     void rotateBody(Eigen::Quaternionf &q);

     void setBodyScale(Eigen::Vector3f &scale);
     void setBodyScale(Eigen::Vector3f &&scale);

     Eigen::Matrix4f getBodyMatrix() const;

private:
     bool ImportModel(std::string pFile);
     bool ImportTestModel();
private:
     QOpenGLContext *ctx;
     /*
     @brief test func
     */
     void drawCube();

     Eigen::Matrix3f J;
     double mass;

     Eigen::Vector3f scale = {1, 1, 1};

     Eigen::Vector3f postition = {0, 0, 0};
     Eigen::Vector3f velocity = {0, 0, 0};
     Eigen::Vector3f acceleration = {0, 0, 0};

     Eigen::Quaternionf orientation = Eigen::Quaternionf::Identity();
     Eigen::Vector3f angularVelocity = {0, 0, 0};
     Eigen::Vector3f angularAcceleration = {0, 0, 0};
     mutable std::mutex mtx;

     std::vector<Mesh> meshes;
};

#endif // BODY_H
