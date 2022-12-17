#include "glsimulation.h"

#include <iostream>

GlSimulation::GlSimulation(QWidget *parent): QOpenGLWidget(parent)
{
    _world = std::make_shared <World> ();
    {
        std::scoped_lock(_pressedKeyMutex);
        _PressedKey[Qt::Key_W] = false;
        _PressedKey[Qt::Key_S] = false;
        _PressedKey[Qt::Key_A] = false;
        _PressedKey[Qt::Key_D] = false;
        _PressedKey[Qt::Key_Shift] = false;
        _PressedKey[Qt::Key_Space] = false;
        _PressedKey[Qt::Key_Q] = false;
        _PressedKey[Qt::Key_E] = false;
    }
    grabKeyboard();
    grabMouse();
}

GlSimulation::~GlSimulation()
{
    _isPaintThreadRun = false;
    _paintThreadHandle.join();
}

void GlSimulation::initializeGL()
{
    initializeOpenGLFunctions();

    createShaderProgramFromFiles(_shaderProgrammBody, "../resources/shaders/vertex_shader.vert", "../resources/shaders/fragment_shader.frag");
    createShaderProgramFromFiles(_shaderProgrammBody, "../resources/shaders/shadow_map_vertex.vert", "../resources/shaders/shadow_map_frag.frag");

    float PlanetScaleFactor = 1.0 / 299792458.0 * 1e0;
    float DisctaneScaleFactor = 1.0 / 299792458.0 * 5e-2;
    float BodyScaleFactor = 1.0 / 299792458.0 * 1e7;

    Body Sun(this->context(), "../resources/models/Sun/Sun.obj");
    float SunScale = 695700 * 1e3 * PlanetScaleFactor;
    Sun.setBodyScale({SunScale, SunScale, SunScale});


    Body Mercury(this->context(), "../resources/models/Mercury/Mercury.obj");
    float MercuryScale = 2439.7 * 1e3 * PlanetScaleFactor;
    Mercury.setBodyScale({MercuryScale, MercuryScale, MercuryScale});

    float MercuryPos = 69816900 * 1e3 * DisctaneScaleFactor;
    Eigen::Vector3d MercuryPosVec({MercuryPos, 0, 0});
    Mercury.setBodyPosition(MercuryPosVec);

    Body Earth(this->context(), "../resources/models/earth.obj");
    //6371000m to the light mseconds 299792458
    float EarthScale = 6371 * 1e3 * PlanetScaleFactor;
    Earth.setBodyScale({EarthScale, EarthScale, EarthScale});

    float EarthPos = 149.6 * 1e6 * 1e3 * DisctaneScaleFactor;
    Eigen::Vector3d EarthPosVec({EarthPos, 0, 0});
    Earth.setBodyPosition(EarthPosVec);

    Body Moon(this->context(), "../resources/models/Moon/Moon.obj");
    float MoonScale = 1737.4 * 1e3 * PlanetScaleFactor;
    Moon.setBodyScale({MoonScale, MoonScale, MoonScale});

    float MoonPosRelativeEarth = 400000 * 1e3 * DisctaneScaleFactor;
    Eigen::Vector3d MoonPosRelativeEarthVec({MoonPosRelativeEarth, 0, 0});
    Moon.setBodyPosition(MoonPosRelativeEarthVec + EarthPosVec);

    Body Neptune(this->context(), "../resources/models/Neptune/Neptune.obj");
    float NeptuneScale = 24622 * 1e3 * PlanetScaleFactor;
    Neptune.setBodyScale({NeptuneScale, NeptuneScale, NeptuneScale});

    float NeptunePos = 4503443661.0 * 1e3 * DisctaneScaleFactor;
    Eigen::Vector3d NeptunePosVec({NeptunePos, 0, 0});
    Neptune.setBodyPosition(NeptunePosVec);

    Body Cubesat6u(this->context(), "../resources/models/CubSat6U.obj");
    // by default scale is mm
    float CubesatScale = 1e-3 * BodyScaleFactor;
    Cubesat6u.setBodyScale({CubesatScale, CubesatScale, CubesatScale});
    Eigen::Vector3d Cubesat6uPosVec = Eigen::Vector3d(500 * 1e3, 0, 0) * DisctaneScaleFactor + EarthPosVec + Eigen::Vector3d(EarthScale, 0, 0);
    Cubesat6u.setBodyPosition(Cubesat6uPosVec);

    _world->_bodies.emplace_back(Cubesat6u);

    _world->_bodies.emplace_back(Sun);
    _world->_bodies.emplace_back(Mercury);
    _world->_bodies.emplace_back(Earth);
    _world->_bodies.emplace_back(Moon);
    _world->_bodies.emplace_back(Neptune);

    _cam.setTranslationCam(Earth.getBodyPosition());

    glClearDepth(1.f);
    glClearColor(0.0f, 0.0f, 0.0f, 0.f);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    _paintThreadHandle = std::thread(&GlSimulation::paintThreadfoo, this);
}

void GlSimulation::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _shaderProgrammBody.bind();

    Eigen::Matrix4f v = _cam.getCameraMatrix().cast <float>();
    Eigen::Matrix4f p = _cam.getProjetionMatrix().cast <float>();

    Eigen::Vector3d camPos = _cam.getTranslation();

    for(auto& body: _world->_bodies)
    {
        body.draw(_shaderProgrammBody, v, p, camPos);
    }
}

void GlSimulation::paintThreadfoo()
{
    using namespace std::literals::chrono_literals;

    Eigen::Vector3d translation {0, 0, 0};
    Eigen::Vector3d max {1e7 / 299792458.0 * 1e1, 1e7 / 299792458.0 * 1e1, 1e7 / 299792458.0 * 1e1};
    max *= (1.0 / 150.0);
    // max *= 10;
    Eigen::Vector3d target {0, 0, 0};
    const double step = 0.15;
    const double C = 0.1;

    float ang_rot = 0.5;

    while(_isPaintThreadRun)
    {
        {
            std::scoped_lock(_pressedKeyMutex);

            if(_PressedKey[Qt::Key_W] == true)
            {
                target[2] = max[2];
                // cam.TranslateCam(Eigen::Vector3f(0, 0,  translation[2]));
            }
            if(_PressedKey[Qt::Key_S] == true)
            {
                target[2] = -max[2];
                // cam.TranslateCam(Eigen::Vector3f(0, 0, translation[2]));
            }
            if(_PressedKey[Qt::Key_A] == true)
            {
                target[0] = max[0];
                // cam.TranslateCam(Eigen::Vector3f(translation[0] , 0, 0));
            }
            if(_PressedKey[Qt::Key_D] == true)
            {
                target[0] = -max[0];
                // cam.TranslateCam(Eigen::Vector3f(translation[0], 0, 0.0));
            }
            if(_PressedKey[Qt::Key_Shift] == true)
            {
                target[1] = max[1];
                // cam.TranslateCam(Eigen::Vector3f(0, translation[1], 0));
            }
            if(_PressedKey[Qt::Key_Space] == true)
            {
                target[1] = -max[1];
                // cam.TranslateCam(Eigen::Vector3f(0, translation[1], 0));
            }
            if(_PressedKey[Qt::Key_Q] == true)
            {
                float angle = ang_rot * M_PI / 180.0f;
                _cam.rotateCam(Eigen::Quaterniond(cos(angle / 2), 0, 0, sin(angle / 2)));
            }
            if(_PressedKey[Qt::Key_E] == true)
            {
                float angle = -ang_rot * M_PI / 180.0f;
                _cam.rotateCam(Eigen::Quaterniond(cos(angle / 2), 0, 0, sin(angle / 2)));
            }
        }
        translation = (translation + target * C * step) / (1 + C * step);
        target = {0, 0, 0};
        std::cout << std::fixed;
        std::cout.precision(10);
        std::cout.width(20);
        // std::cout << translation.transpose() << " " << cam.getTranslation().transpose() << "\r\n";
        _cam.TranslateCam(translation);

        update();
        std::this_thread::sleep_for(15ms);
    }
}

void GlSimulation::keyPressEvent(QKeyEvent *event)
{
    if(event->isAutoRepeat())
        return;
    std::scoped_lock(_pressedKeyMutex);
    int key = event->key();
    _PressedKey[key] = true;
}

void GlSimulation::keyReleaseEvent(QKeyEvent *event)
{
    if(event->isAutoRepeat())
         return;
    std::scoped_lock(_pressedKeyMutex);
    int key = event->key();
    _PressedKey[key] = false;
}

void GlSimulation::mouseMoveEvent(QMouseEvent *event)
{
    int key = event->buttons();
    if((key & Qt::RightButton)  == Qt::RightButton)
    {
        Eigen::Vector2d vec(event->pos().x(), event->pos().y());
        // vec.normalize();
        _cam.rotateCam(vec);
    }
}

void GlSimulation::mousePressEvent(QMouseEvent *event)
{

    int key = event->buttons();
    if((key & Qt::RightButton)  == Qt::RightButton)
    {
        Eigen::Vector2d vec = Eigen::Vector2d(event->pos().x(), event->pos().y());
        _cam.StartRotation(vec);
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
    _cam.setAspectRatio( static_cast<float>(width) / static_cast<float>(height));
    glViewport(0, 0, GLint(width), GLint(height));
}