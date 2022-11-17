#include "glsimulation.h"

#include <iostream>

GlSimulation::GlSimulation(QWidget *parent): QOpenGLWidget(parent)
{
    world = std::make_shared <World> ();
    {
        std::scoped_lock(pressedKeyMutex);
        PressedKey[Qt::Key_W] = false;
        PressedKey[Qt::Key_S] = false;
        PressedKey[Qt::Key_A] = false;
        PressedKey[Qt::Key_D] = false;
        PressedKey[Qt::Key_Shift] = false;
        PressedKey[Qt::Key_Space] = false;
        PressedKey[Qt::Key_Q] = false;
        PressedKey[Qt::Key_E] = false;
    }
    grabKeyboard();
    grabMouse();
}

GlSimulation::~GlSimulation()
{
    isPaintThreadRun = false;
    paintThreadHandle.join();
}

void GlSimulation::initializeGL()
{
    initializeOpenGLFunctions();

    world->bodies.emplace_back(this->context(), "../resources/models/earth.obj");

    world->bodies.emplace_back(this->context(), "../resources/Cube.stl");

    Eigen::Vector3f transl(2, 0, 0);
    world->bodies[1].translateBody(transl);

    createShaderProgramFromFiles(shaderProgramm, "../resources/vertex_shader.vert", "../resources/fragment_shader.frag");

    glClearDepth(1.f);
    glClearColor(0.3f, 0.3f, 0.3f, 0.f);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    paintThreadHandle = std::thread(&GlSimulation::paintThreadfoo, this);
}

void GlSimulation::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaderProgramm.bind();

    float angle = 0.5 * M_PI / 180.0f;
    Eigen::Quaternionf q(cos(angle / 2), 0, 0, sin(angle / 2));
    world->bodies[1].rotateBody(q);

    Eigen::Matrix4f mvp = cam.getCameraProjectiveMatrix();

    for(auto& body: world->bodies)
    {
        body.draw(shaderProgramm, mvp);
    }
}

void GlSimulation::paintThreadfoo()
{
    using namespace std::literals::chrono_literals;

    while(isPaintThreadRun)
    {
        {
            std::scoped_lock(pressedKeyMutex);

            if(PressedKey[Qt::Key_W] == true)
            {
                cam.TranslateCam(Eigen::Vector3f(0, 0,  0.05));
            }
            if(PressedKey[Qt::Key_S] == true)
            {
                cam.TranslateCam(Eigen::Vector3f(0, 0, -0.05));
            }
            if(PressedKey[Qt::Key_A] == true)
            {
                cam.TranslateCam(Eigen::Vector3f(0.05 , 0, 0));
            }
            if(PressedKey[Qt::Key_D] == true)
            {
                cam.TranslateCam(Eigen::Vector3f(-0.05, 0, 0.0));
            }
            if(PressedKey[Qt::Key_Shift] == true)
            {
                cam.TranslateCam(Eigen::Vector3f(0, 0.05, 0));
            }
            if(PressedKey[Qt::Key_Space] == true)
            {
                cam.TranslateCam(Eigen::Vector3f(0, -0.05, 0));
            }
            if(PressedKey[Qt::Key_Q] == true)
            {
                float angle = 0.5 * M_PI / 180.0f;
                cam.rotateCam(Eigen::Quaternionf(cos(angle / 2), 0, 0, sin(angle / 2)));
            }
            if(PressedKey[Qt::Key_E] == true)
            {
                float angle = -0.5 * M_PI / 180.0f;
                cam.rotateCam(Eigen::Quaternionf(cos(angle / 2), 0, 0, sin(angle / 2)));
            }
        }
        update();
        std::this_thread::sleep_for(15ms);
    }
}

void GlSimulation::keyPressEvent(QKeyEvent *event)
{
    if(event->isAutoRepeat())
        return;
    std::scoped_lock(pressedKeyMutex);
    int key = event->key();
    PressedKey[key] = true;
}

void GlSimulation::keyReleaseEvent(QKeyEvent *event)
{
    if(event->isAutoRepeat())
         return;
    std::scoped_lock(pressedKeyMutex);
    int key = event->key();
    PressedKey[key] = false;
}

void GlSimulation::mouseMoveEvent(QMouseEvent *event)
{
    int key = event->buttons();
    if((key & Qt::RightButton)  == Qt::RightButton)
    {
        Eigen::Vector2f vec(event->pos().x(), event->pos().y());
        // vec.normalize();
        cam.rotateCam(vec);
    }
}

void GlSimulation::mousePressEvent(QMouseEvent *event)
{

    int key = event->buttons();
    if((key & Qt::RightButton)  == Qt::RightButton)
    {
        Eigen::Vector2f vec = Eigen::Vector2f(event->pos().x(), event->pos().y());
        cam.StartRotation(vec);
    }
}

void GlSimulation::mouseReleaseEvent(QMouseEvent *event)
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
    cam.setAspectRatio( static_cast<float>(width) / static_cast<float>(height));
    glViewport(0, 0, GLint(width), GLint(height));
}
