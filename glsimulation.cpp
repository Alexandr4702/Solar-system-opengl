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

    float SizeScaleFactor = 1.0 / 299792458.0 * 1e2;
    float DisctaneScaleFactor = 1.0 / 299792458.0 * 1e2;

    Body Cubesat6u(this->context(), "../resources/models/CubSat6U.obj");
    // by default is mm
    float CubesatScale = 1 * SizeScaleFactor;
    Cubesat6u.setBodyScale({CubesatScale, CubesatScale, CubesatScale});

    Body Earth(this->context(), "../resources/models/earth.obj");
    //6371000m to the light mseconds 299792458
    float EarthScale = 6371 * 1e3 * SizeScaleFactor;
    Earth.setBodyScale({EarthScale, EarthScale, EarthScale});

    float EarthPos = 149.6 * 1e6 * 1e3 * SizeScaleFactor;
    Eigen::Vector3f EarthPosVec({EarthPos, 0, 0});
    // Earth.setBodyPosition(EarthPosVec);

    Body Sun(this->context(), "../resources/models/Sun/Sun.obj");
    float SunScale = 695700 * 1e3 * SizeScaleFactor;
    Sun.setBodyScale({SunScale, SunScale, SunScale});

    Body Neptune(this->context(), "../resources/models/Neptune/Neptune.obj");
    float NeptuneScale = 24622 * 1e3 * SizeScaleFactor;
    Neptune.setBodyScale({NeptuneScale, NeptuneScale, NeptuneScale});

    world->bodies.emplace_back(Cubesat6u);

    // world->bodies.emplace_back(Earth);
    world->bodies.emplace_back(Sun);
    // world->bodies.emplace_back(Neptune);


    // cam.setTranslationCam(EarthPosVec);

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

    Eigen::Vector3f translation {0, 0, 0};
    const Eigen::Vector3f max {1e2 / 299792458.0 * 1e1, 1e2 / 299792458.0 * 1e1, 1e2 / 299792458.0 * 1e1};
    Eigen::Vector3f target {0, 0, 0};
    const float step = 0.15;
    const float C = 0.05;

    float ang_rot = 0.5;

    while(isPaintThreadRun)
    {
        {
            std::scoped_lock(pressedKeyMutex);

            if(PressedKey[Qt::Key_W] == true)
            {
                target[2] = max[2];
                // cam.TranslateCam(Eigen::Vector3f(0, 0,  translation[2]));
            }
            if(PressedKey[Qt::Key_S] == true)
            {
                target[2] = -max[2];
                // cam.TranslateCam(Eigen::Vector3f(0, 0, translation[2]));
            }
            if(PressedKey[Qt::Key_A] == true)
            {
                target[0] = max[0];
                // cam.TranslateCam(Eigen::Vector3f(translation[0] , 0, 0));
            }
            if(PressedKey[Qt::Key_D] == true)
            {
                target[0] = -max[0];
                // cam.TranslateCam(Eigen::Vector3f(translation[0], 0, 0.0));
            }
            if(PressedKey[Qt::Key_Shift] == true)
            {
                target[1] = max[1];
                // cam.TranslateCam(Eigen::Vector3f(0, translation[1], 0));
            }
            if(PressedKey[Qt::Key_Space] == true)
            {
                target[1] = -max[1];
                // cam.TranslateCam(Eigen::Vector3f(0, translation[1], 0));
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
        translation = (translation + target * C * step) / (1 + C * step);
        target = {0, 0, 0};
        std::cout << std::fixed;
        std::cout.precision(10);
        std::cout.width(20);
        // std::cout << translation.transpose() << " " << cam.getTranslation().transpose() << "\r\n";
        cam.TranslateCam(translation);

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