#ifndef GLSIMULATION_H
#define GLSIMULATION_H

#include <QOpenGLWidget>
#include <QtOpenGL>
#include <QOpenGLFunctions>

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <string>

#include "world.h"

class GlSimulation: public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT
public:
    GlSimulation(QWidget *parent);
    ~GlSimulation();
protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void initShader();
private:
    bool createShaderProgramFromFiles(QOpenGLShaderProgram& shaderProgramm_, std::string vertesShader, std::string fragmentShader);
    std::shared_ptr <World> world;
    QOpenGLShaderProgram shaderProgramm;

};

#endif // GLSIMULATION_H
