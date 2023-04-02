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
#include <utility>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <eigen/Eigen/Core>
#include <eigen/Eigen/Geometry>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/detail/file_parser_error.hpp>
#include <boost/program_options.hpp>

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
          vertices    = std::move(oth.vertices);
          indices     = std::move(oth.indices);
          raw_texture = std::move(oth.raw_texture);

          arrayBuf = std::move(oth.arrayBuf);
          indexBuf = std::move(oth.indexBuf);
          texture =  std::move(oth.texture);

          ambient_texture  = std::move(oth.ambient_texture );
          diffuse_texture  = std::move(oth.diffuse_texture );
          specular_texture = std::move(oth.specular_texture);
          shininess = oth.shininess;

          ctx = oth.ctx;
          oth.ctx = nullptr;
     }

     Mesh& operator=(Mesh&& oth)
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

          vertices    = std::move(oth.vertices);
          indices     = std::move(oth.indices);
          raw_texture = std::move(oth.raw_texture);

          arrayBuf = std::move(oth.arrayBuf);
          indexBuf = std::move(oth.indexBuf);
          texture =  std::move(oth.texture);

          ambient_texture  = std::move(oth.ambient_texture );
          diffuse_texture  = std::move(oth.diffuse_texture );
          specular_texture = std::move(oth.specular_texture);
          shininess = oth.shininess;

          ctx = oth.ctx;
          oth.ctx = nullptr;

          return *this;
     }

     Mesh(const Mesh& oth): QOpenGLFunctions(oth.ctx),
     indexBuf( new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer )),
     arrayBuf( new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer))
     {
          vertices = oth.vertices;
          indices  = oth.indices;
          raw_texture = oth.raw_texture;
          ambient_texture  = oth.ambient_texture ;
          diffuse_texture  = oth.diffuse_texture ;
          specular_texture = oth.specular_texture;

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

          ambient_texture  = oth.ambient_texture ;
          diffuse_texture  = oth.diffuse_texture ;
          specular_texture = oth.specular_texture;

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

     Eigen::Vector3f ambient_texture = Eigen::Vector3f(0.1, 0.1, 0.1);
     Eigen::Vector3f diffuse_texture = Eigen::Vector3f(0.45, 0.45, 0.45);
     Eigen::Vector3f specular_texture = Eigen::Vector3f(0.45, 0.45, 0.45);
     float shininess = 8;

     private:
     QOpenGLContext *ctx;
};

/*
@brief Class contains information about body and his 3d model.
*/
class Body
{
public:

     enum class RenderType: uint8_t {
          SHADOW_RENDER,
          NORMAL_RENDER,
     };

     struct BodyKinematicParametrs {
          Eigen::Vector3d postition = {0, 0, 0};
          Eigen::Vector3d velocity = {0, 0, 0};
          Eigen::Vector3d acceleration = {0, 0, 0};
          Eigen::Vector3d force = {0, 0, 0};

          Eigen::Quaterniond orientation = Eigen::Quaterniond::Identity();
          Eigen::Vector3d angularVelocity = {0, 0, 0};
          Eigen::Vector3d angularAcceleration = {0, 0, 0};
          Eigen::Vector3d torque = {0, 0, 0};
     };

     Body(QOpenGLContext*);
     Body(QOpenGLContext*, std::string filename, std::string objectName);
     Body(Body&&);
     Body& operator=(Body&&);
     Body(const Body&);
     Body& operator=(const Body&);
     ~Body();
     void draw(QOpenGLShaderProgram& program, RenderType type = RenderType::NORMAL_RENDER);
     void update();

     void setBodyPosition(Eigen::Vector3d&);
     void setBodyPosition(Eigen::Vector3d&&);
     void translateBody(Eigen::Vector3d&);

     const Eigen::Vector3d& getBodyPosition() const;
     Eigen::Vector3d getBodyTranslationMetr() const;

     const BodyKinematicParametrs& getBodyKinematicParametrs();

     void setBodyKinematicParametrs(const BodyKinematicParametrs&);
     void setBodyKinematicParametrs(BodyKinematicParametrs&& );

     void setBodyRotation(Eigen::Quaterniond& q);
     void rotateBody(Eigen::Quaterniond& q);

     void setBodyScale(Eigen::Vector3d& scale);
     void setBodyScale(Eigen::Vector3d&& scale);

     Eigen::Matrix4f getBodyMatrix() const;

     const std::string& getName() const {
          return name;
     }

     const double& getMass () const {return mass;}

private:
     bool ImportModel(std::string pFile);
     bool ImportTestModel();
private:
     QOpenGLContext *ctx;

     Eigen::Matrix3d m_J;
     double mass;
     std::string name;

     Eigen::Vector3d scale = {1, 1, 1};

     //lihgt seconds 299792458
     float positionToMetr = 1;

     BodyKinematicParametrs m_KinematicParametrs;

     mutable std::mutex mtx;
     bool m_castsShadows = true;

     std::vector<Mesh> meshes;
};

#endif // BODY_H
