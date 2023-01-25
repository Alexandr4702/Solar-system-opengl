#ifndef __SHADOWMAP_H__
#define __SHADOWMAP_H__

#include <QOpenGLShaderProgram>
#include <QGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
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

class CascadedShadowMapFBO: public QOpenGLExtraFunctions
{
public:
    CascadedShadowMapFBO(QOpenGLContext* ctx_);

    ~CascadedShadowMapFBO();

    bool Init(unsigned int WindowWidth, unsigned int WindowHeight);

    void BindForWriting(uint CascadeIndex);

    void BindForReading(GLenum TextureUnit);

    void resize(int WindowWidth, int WindowHeight);

private:
    GLuint m_depthMapFBO;
    GLuint m_depthCubemap;
    uint32_t m_width, m_height;
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
        _projectiveMatrixLocation = _shaderProgramTechMap.uniformLocation("projective_matrix");

        if (_lightMatrixLocation < 0 ||
            _projectiveMatrixLocation < 0
            ) {
            std::cerr << "Unable to find location\n";
            return false;
        }
        return true;
    }

    void setMatrixes(Eigen::Matrix4f& lightMatrix, Eigen::Matrix4f& prjectiveMatrix) {
        setLightMatrix(lightMatrix);
        setProjectiveMatrix(prjectiveMatrix);
    }

    void setLightMatrix(Eigen::Matrix4f& lightMatrix)
    {
        if(_lightMatrixLocation >= 0)
            glUniformMatrix4fv(_lightMatrixLocation, 1, GL_FALSE, (const GLfloat*)lightMatrix.data());
    }

    void setProjectiveMatrix(Eigen::Matrix4f& prjectiveMatrix)
    {
        if(_projectiveMatrixLocation >= 0)
            glUniformMatrix4fv(_projectiveMatrixLocation, 1, GL_FALSE, (const GLfloat*)prjectiveMatrix.data());
    }

    QOpenGLShaderProgram _shaderProgramTechMap;
    int _lightMatrixLocation;
    int _projectiveMatrixLocation;

    uint _width ;
    uint _height;
};

class PointShadowMapTech: public QOpenGLFunctions
{
    public:
    PointShadowMapTech(QOpenGLContext* ctx_): QOpenGLFunctions(ctx_)
    {
    }
    bool init()
    {
        return false;
    }

    QOpenGLShaderProgram _shaderProgramTechMap;

    uint _width ;
    uint _height;
};

#endif // __SHADOWMAP_H__