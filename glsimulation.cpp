#include "glsimulation.h"

#include <iostream>

GlSimulation::GlSimulation(QWidget *parent): QOpenGLWidget(parent)
{
    world = std::make_shared <World> ();
    grabKeyboard();
    grabMouse();
}

GlSimulation::~GlSimulation()
{

}

void GlSimulation::initializeGL()
{
    initializeOpenGLFunctions();

    world->bodies.emplace_back(this->context());
    createShaderProgramFromFiles(shaderProgramm, "../resources/vertex_shader.vert", "../resources/fragment_shader.frag");

    glClearDepth(1.f);
    glClearColor(0.3f, 0.3f, 0.3f, 0.f);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

void GlSimulation::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaderProgramm.bind();
    Eigen::Matrix4f mvp = cam.getCameraProjectiveMatrix().transpose();
    // mvp[0][0];
    QMatrix4x4 matrix(mvp.data());
    shaderProgramm.setUniformValue("mvp_matrix", matrix);

    for(auto& body: world->bodies)
    {
        body.draw(shaderProgramm, cam);
    }

    update();
}

void GlSimulation::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    switch(key)
    {
        case Qt::Key_W:
        {
            static int cnt = 0;
            cam.TranslateCam(Eigen::Vector3f(0, 0, 0.05));
        }break;
        case Qt::Key_S:
        {
            cam.TranslateCam(Eigen::Vector3f(0, 0, -0.05));
        }break;
        case Qt::Key_A:
        {
            cam.TranslateCam(Eigen::Vector3f(0.05, 0, 0));
        }break;
        case Qt::Key_D:
        {
            cam.TranslateCam(Eigen::Vector3f(-0.05, 0, 0));
        }break;
        case Qt::Key_Shift:
        {
            cam.TranslateCam(Eigen::Vector3f(0, 0.05, 0));
        }break;
        case Qt::Key_Space:
        {
           cam.TranslateCam(Eigen::Vector3f(0, -0.05, 0));
        }break;
    }
    std::cout << "Key pressed " << cam.getTranslation().transpose() << "\n";
    std::cout << "Matrix \n" << cam.getCameraProjectiveMatrix() << "\n";
}

void GlSimulation::keyReleaseEvent(QKeyEvent *event)
{
    int key = event->key();
    switch(key)
    {
        case Qt::Key_W:
        {
            std::cout << "Key Released \n";
        }break;
        case Qt::Key_S:
        {
        }break;
        case Qt::Key_A:
        {
        }break;
        case Qt::Key_D:
        {
        }break;
        case Qt::Key_Shift:
        {
        }break;
        case Qt::Key_Space:
        {
            std::cout << "W released \n";
        }break;
    }
}

void GlSimulation::mouseMoveEvent(QMouseEvent *event)
{
    int key = event->buttons();
    if((key & Qt::RightButton)  == Qt::RightButton)
    {
        Eigen::Vector2f vec(event->pos().x(), event->pos().y());
        // vec.normalize();
        cam.rotateCam(vec);
        std::cout << "mouse Pressed " << vec.x() << "\n";
    }
}

void GlSimulation::mousePressEvent(QMouseEvent *event)
{

    int key = event->buttons();
    if((key & Qt::RightButton)  == Qt::RightButton)
    {
        Eigen::Vector2f vec = Eigen::Vector2f(event->pos().x(), event->pos().y());
        // vec.normalize();
        cam.StartRotation(vec);
        std::cout << "mouse Pressed " << vec.x() << "\n";
    }
}

void GlSimulation::mouseReleaseEvent(QMouseEvent *event)
{

}

void GlSimulation::drawThread()
{

}

bool GlSimulation::createShaderProgramFromFiles(QOpenGLShaderProgram& shaderProgramm_, std::string vertesShaderPath, std::string fragmentShaderPath)
{
    using namespace std;

    QOpenGLShaderProgram ret;

    QOpenGLShader vertexShader(QOpenGLShader::Vertex);
    QOpenGLShader fragmentShader(QOpenGLShader::Fragment);

    if(!vertexShader.compileSourceFile(vertesShaderPath.c_str()))
    {
        cout << vertexShader.log().toStdString() << " asd\n";
    }

    if(!fragmentShader.compileSourceFile(fragmentShaderPath.c_str()))
    {
        cout << fragmentShader.log().toStdString() << " asd\n";
    }

    if(!shaderProgramm_.addShader(&vertexShader))
    {
        cout << shaderProgramm_.log().toStdString() << " Shader compile programm problem\n";
    }

    if(!shaderProgramm_.addShader(&fragmentShader))
    {
        cout << shaderProgramm_.log().toStdString() << " Shader compile programm problem\n";
    }

    if(!shaderProgramm_.link())
    {
        cout << shaderProgramm_.log().toStdString() << " Shader compile programm problem\n";
    }
    return true;
}

void GlSimulation::resizeGL(int width, int height)
{
    glViewport(0, 0, GLint(width), GLint(height));
}
