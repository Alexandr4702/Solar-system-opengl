#ifndef BODY_H
#define BODY_H
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix3x3>

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QGLContext>

#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Body
{
public:
    Body();
     void draw();
     void update();
private:
     bool ImportModel(std::string pFile);
     const aiScene* scene;
public:
     QMatrix3x3 J;
     double mass;

     QVector3D postition;
     QVector3D velocity;
     QVector3D acceleration;

     QQuaternion orientation;
     QVector3D angularVelocity;
     QVector3D angularAcceleration;
};

#endif // BODY_H
