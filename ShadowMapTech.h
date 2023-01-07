#ifndef __SHADOWMAP_H__
#define __SHADOWMAP_H__

#include <QOpenGLShaderProgram>
#include <QGLContext>
#include <QOpenGLFunctions>
#include <Eigen/Core>

class ShadowMapTech: public QOpenGLFunctions
{
    public:
    ShadowMapTech(QOpenGLContext* ctx_) : QOpenGLFunctions(ctx_)
    {

    }
    bool init()
    {
        _WVPLocation = _shaderProgramTechMap.uniformLocation("gWVP");
        _textureLocation = _shaderProgramTechMap.uniformLocation("gShadowMap");

        if (_WVPLocation == -1 ||
            _textureLocation == -1) {
            return false;
        }

        return true;
    }
    void setWVP(Eigen::Matrix4f& mvp)
    {
        //Should i transpose mstrix??
        glUniformMatrix4fv(_WVPLocation, 1, GL_TRUE, (const GLfloat*)mvp.data());
    }
    void SetTextureUnit(uint32_t TextureUnit)
    {
        glUniform1i(_textureLocation, TextureUnit);
    }
    QOpenGLShaderProgram _shaderProgramTechMap;
    GLuint _WVPLocation;
    GLuint _textureLocation;
};

#endif // __SHADOWMAP_H__