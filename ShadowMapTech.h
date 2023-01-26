#ifndef __SHADOWMAP_H__
#define __SHADOWMAP_H__

#include <QOpenGLShaderProgram>
#include <QGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <Eigen/Core>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"

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

    void BindForWriting();

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
    ShadowMapTech(QOpenGLContext* ctx_) : QOpenGLFunctions(ctx_), _shadowMapFBO(std::make_unique<ShadowMapFBO>(ctx_))
    {
    }

    bool Init(unsigned int Width, unsigned int Height)
    {
        _shadowMapFBO->Init(Width, Height);

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

    void prepeBeforeGeneratingShadowMap() {
        _shadowMapFBO->BindForWriting();
        glClear(GL_DEPTH_BUFFER_BIT);
        _shaderProgramTechMap.bind();
    }

    void preapeforReadingShadowMap(GLint textueUnit, int shadowMapLocation) {
        // int shadowMapLocation = _shaderProgrammBody.uniformLocation("shadowMap");
        if(shadowMapLocation >= 0) {
            glUniform1i(shadowMapLocation, textueUnit);
            _shadowMapFBO->BindForReading(GL_TEXTURE0 + textueUnit);
        }
    }

    void resize(int WindowWidth, int WindowHeight) {
        _shadowMapFBO->resize(WindowWidth, WindowHeight);
    }

    QOpenGLShaderProgram _shaderProgramTechMap;
    int _lightMatrixLocation;
    int _projectiveMatrixLocation;

    uint _width ;
    uint _height;
    std::unique_ptr<ShadowMapFBO> _shadowMapFBO;
};

class PointShadowMapTech: public QOpenGLFunctions
{
    public:
    PointShadowMapTech(QOpenGLContext* ctx_): QOpenGLFunctions(ctx_), _shadowMapFBO(std::make_unique<CascadedShadowMapFBO>(ctx_))
    {
    }
    bool Init(unsigned int Width, unsigned int Height)
    {
        _width = Width;
        _height = Height;

        _projectiveMatrix = glm::perspective(glm::radians(90.0f), static_cast<float>(_width) / static_cast<float>(_height), _nearPlane,  _farPlane);

        _sidesMatrixes[0] = _projectiveMatrix * glm::lookAt(_lightPos, _lightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f));
        _sidesMatrixes[1] = _projectiveMatrix * glm::lookAt(_lightPos, _lightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f));
        _sidesMatrixes[2] = _projectiveMatrix * glm::lookAt(_lightPos, _lightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f));
        _sidesMatrixes[3] = _projectiveMatrix * glm::lookAt(_lightPos, _lightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f));
        _sidesMatrixes[4] = _projectiveMatrix * glm::lookAt(_lightPos, _lightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f));
        _sidesMatrixes[5] = _projectiveMatrix * glm::lookAt(_lightPos, _lightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f));

        _shadowMapFBO->Init(Width, Height);

        _far_planeLocation = _shaderProgramTechMap.uniformLocation("_far_plane");
        _lightPosLocation = _shaderProgramTechMap.uniformLocation("_lightPos");


        for(int i = 0; i < 6; i++) {
            _shadowMatrixesLocations[i] = _shaderProgramTechMap.uniformLocation(std::string("shadowMatrices[" + std::to_string(i) + "]").data());
        }

        if (_far_planeLocation < 0 ||
            _lightPosLocation < 0
            ) {
            std::cerr << "Unable to find location\n";
            return false;
        }
        return true;

        return false;
    }

    void prepeBeforeGeneratingShadowMap ()
    {
        _shadowMapFBO->BindForWriting();
        glClear(GL_DEPTH_BUFFER_BIT);
        _shaderProgramTechMap.bind();

        if(_far_planeLocation > 0)
            glUniform1f(_far_planeLocation, _farPlane);

        if(_lightPosLocation > 0)
            glUniform3fv(_lightPosLocation, 1,glm::value_ptr(_lightPos));

        for(int i = 0; i < 6; i++) {
            if(_shadowMatrixesLocations[i] > 0) {
                glUniformMatrix4fv(_shadowMatrixesLocations[i], 1, GL_FALSE, glm::value_ptr(_sidesMatrixes[i]));
            }
        }

    }
    void preapeforReadingShadowMap(GLint textueUnit, int shadowMapLocation) {
        // int shadowMapLocation = _shaderProgrammBody.uniformLocation("shadowMap");
        if(shadowMapLocation >= 0) {
            glUniform1i(shadowMapLocation, textueUnit);
            _shadowMapFBO->BindForReading(GL_TEXTURE0 + textueUnit);
        }
    }

    void resize(int Width, int Height) {
        _width = Width;
        _height = Height;
        _projectiveMatrix = glm::perspective(glm::radians(90.0f), static_cast<float>(_width) / static_cast<float>(_height), _nearPlane,  _farPlane);

        _sidesMatrixes[0] = _projectiveMatrix * glm::lookAt(_lightPos, _lightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f));
        _sidesMatrixes[1] = _projectiveMatrix * glm::lookAt(_lightPos, _lightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f));
        _sidesMatrixes[2] = _projectiveMatrix * glm::lookAt(_lightPos, _lightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f));
        _sidesMatrixes[3] = _projectiveMatrix * glm::lookAt(_lightPos, _lightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f));
        _sidesMatrixes[4] = _projectiveMatrix * glm::lookAt(_lightPos, _lightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f));
        _sidesMatrixes[5] = _projectiveMatrix * glm::lookAt(_lightPos, _lightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f));

        _shadowMapFBO->resize(_width, _height);
    }
    QOpenGLShaderProgram _shaderProgramTechMap;
    std::unique_ptr<CascadedShadowMapFBO> _shadowMapFBO;
    glm::mat4 _sidesMatrixes[6];
    glm::mat4 _projectiveMatrix;
    glm::vec3 _lightPos;
    float _farPlane = 25.0;
    float _nearPlane = 1.0;

    int _far_planeLocation = 0;
    int _lightPosLocation = 0;
    int _shadowMatrixesLocations[6] = {0};

    uint _width ;
    uint _height;
};

#endif // __SHADOWMAP_H__