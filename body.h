#ifndef BODY_H
#define BODY_H

#include <QOpenGLShaderProgram>
#include <QGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
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

          indexBuf->setUsagePattern(QOpenGLBuffer::StaticDraw);
          arrayBuf->setUsagePattern(QOpenGLBuffer::StaticDraw);
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

     Mesh& operator=(Mesh&& other)
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

          vertices = move(other.vertices);
          indices  = move(other.indices);
          raw_texture = std::move(other.raw_texture);

          arrayBuf = move(other.arrayBuf);
          indexBuf = move(other.indexBuf);
          texture = move(other.texture);

          ctx = other.ctx;
          other.ctx = nullptr;

          return *this;
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

          indexBuf->setUsagePattern(QOpenGLBuffer::StaticDraw);
          arrayBuf->setUsagePattern(QOpenGLBuffer::StaticDraw);

          arrayBuf->bind();
          arrayBuf->allocate(vertices.data(), vertices.size() * sizeof(vertices[0]));

          indexBuf->bind();
          indexBuf->allocate(indices.data(), indices.size() * sizeof(indices[0]));

          texture = std::shared_ptr<QOpenGLTexture> (new QOpenGLTexture(raw_texture.mirrored()));

          ctx = oth.ctx;
     }

     Mesh& operator=(const Mesh& oth)
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

          vertices = oth.vertices;
          indices  = oth.indices;
          raw_texture = oth.raw_texture;

          arrayBuf->create();
          indexBuf->create();

          indexBuf->setUsagePattern(QOpenGLBuffer::StaticDraw);
          arrayBuf->setUsagePattern(QOpenGLBuffer::StaticDraw);

          arrayBuf->bind();
          arrayBuf->allocate(vertices.data(), vertices.size() * sizeof(vertices[0]));

          indexBuf->bind();
          indexBuf->allocate(indices.data(), indices.size() * sizeof(indices[0]));

          texture = std::shared_ptr<QOpenGLTexture> (new QOpenGLTexture(raw_texture.mirrored()));

          ctx = oth.ctx;

          return *this;
     }

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

     Eigen::Vector3f ambient_texture;
     Eigen::Vector3f diffuse_texture;
     Eigen::Vector3f specular_texture;
     float shininess;

     private:
     QOpenGLContext *ctx;
};

/*
@brief Class contains information about body and his 3d model.
*/
class Body
{
public:
     Body(QOpenGLContext*);
     Body(QOpenGLContext*, std::string filename);
     Body(Body&&);
     Body& operator=(Body&&);
     Body(const Body&);
     Body& operator=(const Body&);
     ~Body();
     void draw(QOpenGLShaderProgram& program);
     void draw(QOpenGLShaderProgram& program, Eigen::Matrix4f& viewMatrix, Eigen::Matrix4f& projectionMatrix, Eigen::Vector3d& camPos);
     void update();

     void setBodyPosition(Eigen::Vector3d&);
     void setBodyPosition(Eigen::Vector3d&&);
     void translateBody(Eigen::Vector3d&);

     Eigen::Vector3d getBodyPosition() const;
     Eigen::Vector3d getBodyTranslationMetr() const;

     void setBodyRotation(Eigen::Quaterniond& q);
     void rotateBody(Eigen::Quaterniond& q);

     void setBodyScale(Eigen::Vector3d& scale);
     void setBodyScale(Eigen::Vector3d&& scale);

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

     Eigen::Matrix3d J;
     double mass;

     Eigen::Vector3d scale = {1, 1, 1};

     //lihgt seconds 299792458
     Eigen::Vector3d postition = {0, 0, 0};
     float positionToMetr = 1;
     //lihgt seconds/s 299792458
     Eigen::Vector3d velocity = {0, 0, 0};
     Eigen::Vector3d acceleration = {0, 0, 0};

     Eigen::Quaterniond orientation = Eigen::Quaterniond::Identity();
     Eigen::Vector3d angularVelocity = {0, 0, 0};
     Eigen::Vector3d angularAcceleration = {0, 0, 0};
     mutable std::mutex mtx;

     std::vector<Mesh> meshes;
};

#endif // BODY_H
