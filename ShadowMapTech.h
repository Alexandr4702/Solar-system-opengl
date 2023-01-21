#ifndef __SHADOWMAP_H__
#define __SHADOWMAP_H__

#include <QOpenGLShaderProgram>
#include <QGLContext>
#include <QOpenGLFunctions>
#include <Eigen/Core>
#include <iostream>

class ShadowMapFBO: public QOpenGLFunctions
{
public:
    ShadowMapFBO(QOpenGLContext* ctx_);

    ~ShadowMapFBO();

    bool Init(unsigned int Width, unsigned int Height, bool ForPCF = false);

    void BindForWriting();

    void BindForReading(GLenum TextureUnit);

private:
    uint m_width = 0;
    uint m_height = 0;
    GLuint m_fbo;
    GLuint m_shadowMap;
};

class CascadedShadowMapFBO: public QOpenGLFunctions
{
public:
    CascadedShadowMapFBO();

    ~CascadedShadowMapFBO();

    bool Init(unsigned int WindowWidth, unsigned int WindowHeight);

    void BindForWriting(uint CascadeIndex);

    void BindForReading();

private:
    GLuint m_fbo;
    GLuint m_shadowMap[3];
};

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

        if (_WVPLocation < 0 ||
            _textureLocation < 0) {
            std::cerr << "Unable to find location\n";
            return false;
        }

        return true;
    }
    void setWVP(Eigen::Matrix4f& mvp)
    {
        // Should i transpose mstrix??
        if(_WVPLocation >= 0)
            glUniformMatrix4fv(_WVPLocation, 1, GL_TRUE, (const GLfloat*)mvp.data());
    }
    void SetTextureUnit(uint32_t TextureUnit)
    {
        if(_textureLocation >= 0)
            glUniform1i(_textureLocation, TextureUnit);
    }
    QOpenGLShaderProgram _shaderProgramTechMap;
    int _WVPLocation;
    int _textureLocation;

    uint _width ;
    uint _height;
};

#endif // __SHADOWMAP_H__