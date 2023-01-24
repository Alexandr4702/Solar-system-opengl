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

    void resize(int WindowWidth, int WindowHeight);

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

    void resize(int WindowWidth, int WindowHeight);

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
        _lightMatrixLocation = _shaderProgramTechMap.uniformLocation("light_matrix");
        _textureLocation = _shaderProgramTechMap.uniformLocation("gShadowMap");
        _projectiveMatrixLocation = _shaderProgramTechMap.uniformLocation("projective_matrix");

        if (_lightMatrixLocation < 0 ||
            _textureLocation < 0 ||
            _projectiveMatrixLocation < 0
            ) {
            std::cerr << "Unable to find location\n";
            return false;
        }

        return true;
    }
    void setLightMatrix(Eigen::Matrix4f& mvp)
    {
        // Should i transpose matrix??
        if(_lightMatrixLocation >= 0)
            glUniformMatrix4fv(_lightMatrixLocation, 1, GL_TRUE, (const GLfloat*)mvp.data());
    }

    void setProjectiveMatrix(Eigen::Matrix4f& mvp)
    {
        // Should i transpose matrix??
        if(_lightMatrixLocation >= 0)
            glUniformMatrix4fv(_projectiveMatrixLocation, 1, GL_TRUE, (const GLfloat*)mvp.data());
    }

    void SetTextureUnit(uint32_t TextureUnit)
    {
        if(_textureLocation >= 0)
            glUniform1i(_textureLocation, TextureUnit);
    }
    QOpenGLShaderProgram _shaderProgramTechMap;
    int _lightMatrixLocation;
    int _textureLocation;
    int _projectiveMatrixLocation;

    uint _width ;
    uint _height;
};

#endif // __SHADOWMAP_H__