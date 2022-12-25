#ifndef __SHADOWMAP_H__
#define __SHADOWMAP_H__

#include <QOpenGLShaderProgram>
#include <QGLContext>
#include <QOpenGLFunctions>

class ShadowMapTech: public QOpenGLFunctions
{
    public:
    ShadowMapTech(QOpenGLContext* ctx_):QOpenGLFunctions(ctx_)
    {

    }
    QOpenGLShaderProgram shaderProgramTechMap;
};

#endif // __SHADOWMAP_H__