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

class Material : public QOpenGLFunctions
{
public:
     Material(QOpenGLContext *ctx_, aiMaterial *mat, const std::string pathToTextureFolder) : QOpenGLFunctions(ctx_), m_ctx(ctx_), m_name(mat->GetName().data)
     {
          aiColor3D color(0.f, 0.f, 0.f);
          float val;
          aiString texture_path;
          unsigned int texture_index;

          if (mat->Get(AI_MATKEY_COLOR_AMBIENT, color) == aiReturn_SUCCESS)
          {
               m_ambientCoeff = Eigen::Vector3f(color.r, color.g, color.b);
          }
          if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == aiReturn_SUCCESS)
          {
               m_diffuseCoeff = Eigen::Vector3f(color.r, color.g, color.b);
               ;
          }
          if (mat->Get(AI_MATKEY_COLOR_SPECULAR, color) == aiReturn_SUCCESS)
          {
               m_specularCoeff = Eigen::Vector3f(color.r, color.g, color.b);
               ;
          }
          if (mat->Get(AI_MATKEY_SHININESS, val) == aiReturn_SUCCESS)
          {
               m_shininess = val;
          }

          if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path, NULL, &texture_index) == aiReturn_SUCCESS)
          {
               std::string path_to_texture(texture_path.data);
               path_to_texture = pathToTextureFolder + "/" + path_to_texture;
               m_raw_ColorTexture = QImage(path_to_texture.c_str());
               if (m_raw_ColorTexture.isNull())
               {
                    std::cout << "Cannot open texture\n";
               }
               else
               {
                    m_colorTexture = std::shared_ptr<QOpenGLTexture>(new QOpenGLTexture(m_raw_ColorTexture.mirrored()));
               }
          }

          if (mat->GetTexture(aiTextureType_HEIGHT, 0, &texture_path, NULL, &texture_index) == aiReturn_SUCCESS)
          {
               std::string path_to_texture(texture_path.data);
               path_to_texture = pathToTextureFolder + "/" + path_to_texture;
               m_raw_BumpTexture = QImage(path_to_texture.c_str());

               m_colorTexture = std::shared_ptr<QOpenGLTexture>(new QOpenGLTexture(m_raw_ColorTexture.mirrored()));
          }
     }

     Material(const Material &oth) : QOpenGLFunctions(oth.m_ctx), m_ctx(oth.m_ctx)
     {
          m_raw_ColorTexture = oth.m_raw_ColorTexture;
          m_colorTexture = oth.m_colorTexture;

          m_ambientCoeff = oth.m_ambientCoeff;
          m_diffuseCoeff = oth.m_diffuseCoeff;
          m_specularCoeff = oth.m_specularCoeff;
     }

     Material(Material &&oth) : QOpenGLFunctions(oth.m_ctx), m_ctx(oth.m_ctx)
     {
          m_raw_ColorTexture = std::move(oth.m_raw_ColorTexture);
          m_colorTexture = std::move(oth.m_colorTexture);

          m_ambientCoeff = std::move(oth.m_ambientCoeff);
          m_diffuseCoeff = std::move(oth.m_diffuseCoeff);
          m_specularCoeff = std::move(oth.m_specularCoeff);
          m_shininess = oth.m_shininess;
     }

     Material &operator=(const Material &oth)
     {
          return *this;
     }

     Material &operator=(Material &&oth)
     {
          return *this;
     }

     ~Material()
     {
     }

     void bindTexture(QOpenGLShaderProgram &program)
     {
          if (m_colorTexture.get() != nullptr)
          {
               m_colorTexture->bind(3);
               program.setUniformValue("colorTexture", 3);
          }

          if (m_BumpTexture.get() != nullptr)
          {
               // m_displacmentTexture->bind(3);
               // program.setUniformValue("textures", 3);
          }

          QVector3D ambient_texture_QT(m_ambientCoeff.x(), m_ambientCoeff.y(), m_ambientCoeff.z());
          program.setUniformValue("ambient_coeffs", ambient_texture_QT);

          QVector3D diffuse_texture_QT(m_diffuseCoeff.x(), m_diffuseCoeff.y(), m_diffuseCoeff.z());
          program.setUniformValue("diffuse_coeffs", diffuse_texture_QT);

          QVector3D specular_texture_QT(m_specularCoeff.x(), m_specularCoeff.y(), m_specularCoeff.z());
          program.setUniformValue("specular_coeffs", specular_texture_QT);

          float shininess = this->m_shininess;
          program.setUniformValue("shininess", m_shininess);
     }

     void releaseTexture()
     {

          if (m_colorTexture.get() != nullptr)
               m_colorTexture->release(3);

          if (m_BumpTexture.get() != nullptr)
               m_BumpTexture->release(3);
     }

private:
     QOpenGLContext *m_ctx;
     std::string m_name;

     std::shared_ptr<QOpenGLTexture> m_colorTexture = nullptr;
     QImage m_raw_ColorTexture;

     std::shared_ptr<QOpenGLTexture> m_BumpTexture = nullptr;
     QImage m_raw_BumpTexture;

     Eigen::Vector3f m_ambientCoeff = Eigen::Vector3f(0.1, 0.1, 0.1);
     Eigen::Vector3f m_diffuseCoeff = Eigen::Vector3f(0.45, 0.45, 0.45);
     Eigen::Vector3f m_specularCoeff = Eigen::Vector3f(0.45, 0.45, 0.45);
     float m_shininess = 8;
};

class Mesh : public QOpenGLFunctions
{
public:
     struct VertexData
     {
          Eigen::Vector3f position;
          Eigen::Vector2f colorTextCoord;
          Eigen::Vector3f normal;
     };

     Mesh(QOpenGLContext *ctx_) : QOpenGLFunctions(ctx_), m_ctx(ctx_),
                                  m_indexBuf(new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer)),
                                  m_arrayBuf(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer))
     {
          m_arrayBuf->create();
          m_indexBuf->create();

          m_indexBuf->setUsagePattern(QOpenGLBuffer::StaticDraw);
          m_arrayBuf->setUsagePattern(QOpenGLBuffer::StaticDraw);
     }

     Mesh(Mesh &&oth) : QOpenGLFunctions(oth.m_ctx)
     {
          m_vertices = std::move(oth.m_vertices);
          m_indices = std::move(oth.m_indices);

          m_arrayBuf = std::move(oth.m_arrayBuf);
          m_indexBuf = std::move(oth.m_indexBuf);

          m_mat = std::move(oth.m_mat);

          m_ctx = oth.m_ctx;
          oth.m_ctx = nullptr;
     }

     Mesh &operator=(Mesh &&oth)
     {
          m_vertices = std::move(oth.m_vertices);
          m_indices = std::move(oth.m_indices);

          m_arrayBuf = std::move(oth.m_arrayBuf);
          m_indexBuf = std::move(oth.m_indexBuf);

          m_mat = std::move(oth.m_mat);

          m_ctx = oth.m_ctx;
          oth.m_ctx = nullptr;

          return *this;
     }

     Mesh(const Mesh &oth) : QOpenGLFunctions(oth.m_ctx),
                             m_indexBuf(new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer)),
                             m_arrayBuf(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer))
     {
          m_vertices = oth.m_vertices;
          m_indices = oth.m_indices;
          m_mat = oth.m_mat;

          m_arrayBuf->create();
          m_indexBuf->create();

          m_indexBuf->setUsagePattern(QOpenGLBuffer::StaticDraw);
          m_arrayBuf->setUsagePattern(QOpenGLBuffer::StaticDraw);

          m_arrayBuf->bind();
          m_arrayBuf->allocate(m_vertices.data(), m_vertices.size() * sizeof(m_vertices[0]));

          m_indexBuf->bind();
          m_indexBuf->allocate(m_indices.data(), m_indices.size() * sizeof(m_indices[0]));

          m_ctx = oth.m_ctx;
     }

     Mesh &operator=(const Mesh &oth)
     {
          m_vertices = oth.m_vertices;
          m_indices = oth.m_indices;
          m_mat = oth.m_mat;

          m_arrayBuf->create();
          m_indexBuf->create();

          m_indexBuf->setUsagePattern(QOpenGLBuffer::StaticDraw);
          m_arrayBuf->setUsagePattern(QOpenGLBuffer::StaticDraw);

          m_arrayBuf->bind();
          m_arrayBuf->allocate(m_vertices.data(), m_vertices.size() * sizeof(m_vertices[0]));

          m_indexBuf->bind();
          m_indexBuf->allocate(m_indices.data(), m_indices.size() * sizeof(m_indices[0]));

          m_ctx = oth.m_ctx;

          return *this;
     }

     ~Mesh()
     {
     }

     void draw(QOpenGLShaderProgram &program);

     std::vector<VertexData> m_vertices;
     std::vector<uint32_t> m_indices;

     std::shared_ptr<QOpenGLBuffer> m_arrayBuf;
     std::shared_ptr<QOpenGLBuffer> m_indexBuf;

     std::shared_ptr<Material> m_mat;

private:
     QOpenGLContext *m_ctx;
};

/*
@brief Class contains information about body and his 3d model.
*/
class Body
{
public:
     enum class RenderType : uint8_t
     {
          SHADOW,
          NORMAL,
     };

     struct BodyKinematicParametrs
     {
          Eigen::Vector3d postition = {0, 0, 0};
          Eigen::Vector3d velocity = {0, 0, 0};
          Eigen::Vector3d acceleration = {0, 0, 0};
          Eigen::Vector3d force = {0, 0, 0};

          Eigen::Quaterniond orientation = Eigen::Quaterniond::Identity();
          Eigen::Vector3d angularVelocity = {0, 0, 0};
          Eigen::Vector3d angularAcceleration = {0, 0, 0};
          Eigen::Vector3d torque = {0, 0, 0};
     };

     Body(QOpenGLContext *);
     Body(QOpenGLContext *, std::string filename, std::string objectName);
     Body(Body &&);
     Body &operator=(Body &&);
     Body(const Body &);
     Body &operator=(const Body &);
     ~Body();
     void draw(QOpenGLShaderProgram &program, RenderType type = RenderType::NORMAL);
     void update();

     void setBodyPosition(Eigen::Vector3d &);
     void setBodyPosition(Eigen::Vector3d &&);
     void translateBody(Eigen::Vector3d &);

     const Eigen::Vector3d &getBodyPosition() const;
     Eigen::Vector3d getBodyTranslationMetr() const;

     const BodyKinematicParametrs &getBodyKinematicParametrs();

     void setBodyKinematicParametrs(const BodyKinematicParametrs &);
     void setBodyKinematicParametrs(BodyKinematicParametrs &&);

     void setBodyRotation(Eigen::Quaterniond &q);
     void rotateBody(Eigen::Quaterniond &q);

     void setBodyScale(Eigen::Vector3d &scale);
     void setBodyScale(Eigen::Vector3d &&scale);

     Eigen::Matrix4f getBodyMatrix() const;

     const std::string &getName() const
     {
          return name;
     }

     const double &getMass() const { return mass; }

private:
     bool ImportModel(std::string pFile);
     bool ImportTestModel();

private:
     QOpenGLContext *ctx;

     Eigen::Matrix3d m_J;
     double mass;
     std::string name;

     Eigen::Vector3d scale = {1, 1, 1};

     // lihgt seconds 299792458
     float positionToMetr = 1;

     BodyKinematicParametrs m_KinematicParametrs;

     mutable std::mutex m_mtx;
     bool m_castsShadows = true;

     std::vector<Mesh> m_meshes;
};

#endif // BODY_H
