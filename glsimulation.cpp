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

    createShaderProgramFromFiles(shaderProgramm, "../resources/shaders/vertex_shader.vert", "../resources/shaders/fragment_shader.frag");


    Body Cubesat6u(this->context(), "../resources/models/CubSat6U.obj");
    Cubesat6u.setBodyScale({0.001, 0.001, 0.001});

    Body Earth(this->context(), "../resources/models/earth.obj");
    Earth.setBodyPosition({-5, 0, 0});

    Body Sun(this->context(), "../resources/models/Sun/Sun.obj");

    world->bodies.emplace_back(Cubesat6u);

    Cubesat6u.setBodyPosition({3, 0, 0});
    world->bodies.emplace_back(Cubesat6u);

    world->bodies.emplace_back(Earth);

    world->bodies.emplace_back(Sun);


    glClearDepth(1.f);
    glClearColor(0.0f, 0.0f, 0.0f, 0.f);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    paintThreadHandle = std::thread(&GlSimulation::paintThreadfoo, this);
}

void GlSimulation::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaderProgramm.bind();

    Eigen::Matrix4f mvp = cam.getCameraProjectiveMatrix();

    for(auto& body: world->bodies)
    {
        body.draw(shaderProgramm, mvp);
    }
}

void GlSimulation::paintThreadfoo()
{
    using namespace std::literals::chrono_literals;
    float translation = 0.05;
    float ang_rot = 0.5;

    while(isPaintThreadRun)
    {
        {
            std::scoped_lock(pressedKeyMutex);

            if(PressedKey[Qt::Key_W] == true)
            {
                cam.TranslateCam(Eigen::Vector3f(0, 0,  translation));
            }
            if(PressedKey[Qt::Key_S] == true)
            {
                cam.TranslateCam(Eigen::Vector3f(0, 0, -translation));
            }
            if(PressedKey[Qt::Key_A] == true)
            {
                cam.TranslateCam(Eigen::Vector3f(translation , 0, 0));
            }
            if(PressedKey[Qt::Key_D] == true)
            {
                cam.TranslateCam(Eigen::Vector3f(-translation, 0, 0.0));
            }
            if(PressedKey[Qt::Key_Shift] == true)
            {
                cam.TranslateCam(Eigen::Vector3f(0, translation, 0));
            }
            if(PressedKey[Qt::Key_Space] == true)
            {
                cam.TranslateCam(Eigen::Vector3f(0, -translation, 0));
            }
            if(PressedKey[Qt::Key_Q] == true)
            {
                float angle = ang_rot * M_PI / 180.0f;
                cam.rotateCam(Eigen::Quaternionf(cos(angle / 2), 0, 0, sin(angle / 2)));
            }
            if(PressedKey[Qt::Key_E] == true)
            {
                float angle = -ang_rot * M_PI / 180.0f;
                cam.rotateCam(Eigen::Quaternionf(cos(angle / 2), 0, 0, sin(angle / 2)));
            }
        }
        // float angle = 0.5 * M_PI / 180.0f;
        // Eigen::Quaternionf q(cos(angle / 2), 0, 0, sin(angle / 2));
        // world->bodies[1].rotateBody(q);

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