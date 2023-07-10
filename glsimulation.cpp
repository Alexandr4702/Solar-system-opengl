#include "glsimulation.h"

#include <iostream>

namespace pt = boost ::property_tree;
namespace po = boost::program_options;

GlSimulation::GlSimulation(QWidget *parent) : QOpenGLWidget(parent)
{
    _world = std::make_shared<World>();
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
    _isPaintThreadRun.store(false, std::memory_order_relaxed);
    _paintThreadHandle.join();
}

void GlSimulation::initializeGL()
{
    initializeOpenGLFunctions();

    readSettings("../resources/settings/Settings.json");

    _shadowMapTechPtr = std::make_unique<PointShadowMapTech>(context());

    createShaderProgramFromFiles(_shaderProgrammBody,
                                 "../resources/shaders/Body/vertex_shader.vert",
                                 "../resources/shaders/Body/fragment_shader.frag");
    createShaderProgramFromFiles(_shadowMapTechPtr->_shaderProgramTechMap,
                                 "../resources/shaders/shadowMapGeneratorPointLight/shadow_map_vertex.vert",
                                 "../resources/shaders/shadowMapGeneratorPointLight/shadow_map_geometry.geom",
                                 "../resources/shaders/shadowMapGeneratorPointLight/shadow_map_frag.frag");

    if (!_shadowMapTechPtr->Init(size().width(), size().height()))
        std::cerr << "blyatstcvo razvrat narkotiki \n";

    _shadowMapTechPtr->set_FarNear(_cam.getCamParametrs().near_plane, _cam.getCamParametrs().far_plane);

    float PlanetScaleFactor = 1.0 / 299792458.0 * 1e0;
    float DisctaneScaleFactor = 1.0 / 299792458.0 * 5e-2;
    float BodyScaleFactor = 1.0 / 299792458.0 * 1e7;

    readBodiesFromJson("../resources/settings/BodiesList.json");

    glClearDepth(1.f);
    glClearColor(0.0f, 0.0f, 0.0f, 0.f);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    _world->StartSimulations();
    _paintThreadHandle = std::thread(&GlSimulation::paintThreadfoo, this);
}

void GlSimulation::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //--- Prepe data
    Eigen::Matrix4f viewMat = _cam.getCameraMatrix().cast<float>();
    Eigen::Matrix4f projectMat = _cam.getProjetionMatrix().cast<float>();
    Eigen::Vector3f camPos = _cam.getTranslation().cast<float>();

    Eigen::Affine3f LightTransform;
    LightTransform.setIdentity();
    LightTransform.translate(Eigen::Vector3f::Zero());
    Eigen::Matrix4f lightMatrix = LightTransform.matrix();

    //--- Creating shadow map
    _shadowMapTechPtr->prepeBeforeGeneratingShadowMap();
    _shadowMapTechPtr->setMatrixes(lightMatrix, projectMat);

    for (auto &body : _world->_bodies)
    {
        body.draw(_shadowMapTechPtr->_shaderProgramTechMap, Body::RenderType::SHADOW);
    }
    // return;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //---Draw body
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, size().width(), size().height());
    _shaderProgrammBody.bind();

    //---Set Common Variable
    int shadowMapLocation = _shaderProgrammBody.uniformLocation("PointShadowMap");
    int shadowMapTexutreUnit = 13;
    if (shadowMapLocation >= 0)
    {
        glUniform1i(shadowMapLocation, shadowMapTexutreUnit);
        _shadowMapTechPtr->_shadowMapFBO->BindForReading(GL_TEXTURE0 + shadowMapTexutreUnit);
    }

    int far_planeLocaction_BodyShader = _shaderProgrammBody.uniformLocation("far_plane");
    if (far_planeLocaction_BodyShader >= 0)
    {
        glUniform1f(far_planeLocaction_BodyShader, _cam.getCamParametrs().far_plane);
    }

    int near_planeLocaction_BodyShader = _shaderProgrammBody.uniformLocation("near_plane");
    if (near_planeLocaction_BodyShader >= 0)
    {
        glUniform1f(near_planeLocaction_BodyShader, _cam.getCamParametrs().near_plane);
    }

    int lightMatrixLocaction_BodyShader = _shaderProgrammBody.uniformLocation("light_matrix");
    if (lightMatrixLocaction_BodyShader >= 0)
    {
        glUniformMatrix4fv(lightMatrixLocaction_BodyShader, 1, GL_FALSE, (const GLfloat *)lightMatrix.data());
    }

    int viewMatrixLocaction = _shaderProgrammBody.uniformLocation("view_matrix");
    if (viewMatrixLocaction >= 0)
    {
        glUniformMatrix4fv(viewMatrixLocaction, 1, GL_FALSE, (const GLfloat *)viewMat.data());
    }

    int projectiveMatrixLocaction_BodyShader = _shaderProgrammBody.uniformLocation("projective_matrix");
    if (projectiveMatrixLocaction_BodyShader >= 0)
    {
        glUniformMatrix4fv(projectiveMatrixLocaction_BodyShader, 1, GL_FALSE, (const GLfloat *)projectMat.data());
    }

    int camMatrixLocaction = _shaderProgrammBody.uniformLocation("camPosition");
    if (camMatrixLocaction >= 0)
    {
        glUniform3fv(camMatrixLocaction, 1, camPos.data());
    }

    for (auto &body : _world->_bodies)
    {
        body.draw(_shaderProgrammBody);
    }
}

void GlSimulation::paintThreadfoo()
{
    using namespace std::literals::chrono_literals;

    Eigen::Vector3d translation{0, 0, 0};
    Eigen::Vector3d max{1e7 / 299792458.0 * 1e1, 1e7 / 299792458.0 * 1e1, 1e7 / 299792458.0 * 1e1};
    max *= (1.0 / 150.0);
    // max *= 10;
    Eigen::Vector3d target{0, 0, 0};
    const double step = 0.15;
    const double C = 0.1;

    float ang_rot = 0.5;

    while (_isPaintThreadRun.load(std::memory_order_relaxed))
    {
        {
            std::scoped_lock(_pressedKeyMutex);

            if (_PressedKey[Qt::Key_W] == true)
            {
                target[2] = max[2];
                // cam.TranslateCam(Eigen::Vector3f(0, 0,  translation[2]));
            }
            if (_PressedKey[Qt::Key_S] == true)
            {
                target[2] = -max[2];
                // cam.TranslateCam(Eigen::Vector3f(0, 0, translation[2]));
            }
            if (_PressedKey[Qt::Key_A] == true)
            {
                target[0] = max[0];
                // cam.TranslateCam(Eigen::Vector3f(translation[0] , 0, 0));
            }
            if (_PressedKey[Qt::Key_D] == true)
            {
                target[0] = -max[0];
                // cam.TranslateCam(Eigen::Vector3f(translation[0], 0, 0.0));
            }
            if (_PressedKey[Qt::Key_Shift] == true)
            {
                target[1] = max[1];
                // cam.TranslateCam(Eigen::Vector3f(0, translation[1], 0));
            }
            if (_PressedKey[Qt::Key_Space] == true)
            {
                target[1] = -max[1];
                // cam.TranslateCam(Eigen::Vector3f(0, translation[1], 0));
            }
            if (_PressedKey[Qt::Key_Q] == true)
            {
                float angle = ang_rot * M_PI / 180.0f;
                _cam.rotateCam(Eigen::Quaterniond(cos(angle / 2), 0, 0, sin(angle / 2)));
            }
            if (_PressedKey[Qt::Key_E] == true)
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
    if (event->isAutoRepeat())
        return;
    std::scoped_lock(_pressedKeyMutex);
    int key = event->key();
    _PressedKey[key] = true;
}

void GlSimulation::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;
    std::scoped_lock(_pressedKeyMutex);
    int key = event->key();
    _PressedKey[key] = false;
}

void GlSimulation::mouseMoveEvent(QMouseEvent *event)
{
    int key = event->buttons();
    if ((key & Qt::RightButton) == Qt::RightButton)
    {
        Eigen::Vector2d vec(event->pos().x(), event->pos().y());
        // vec.normalize();
        _cam.rotateCam(vec);
    }
}

void GlSimulation::mousePressEvent(QMouseEvent *event)
{

    int key = event->buttons();
    if ((key & Qt::RightButton) == Qt::RightButton)
    {
        Eigen::Vector2d vec = Eigen::Vector2d(event->pos().x(), event->pos().y());
        _cam.StartRotation(vec);
    }
}

void GlSimulation::mouseReleaseEvent(QMouseEvent *event)
{
}

bool GlSimulation::createShaderProgramFromFiles(QOpenGLShaderProgram &shaderProgramm_, std::string vertesShaderPath, std::string fragmentShaderPath)
{
    using namespace std;

    QOpenGLShaderProgram ret;

    QOpenGLShader vertexShader(QOpenGLShader::Vertex);
    QOpenGLShader fragmentShader(QOpenGLShader::Fragment);

    if (!vertexShader.compileSourceFile(vertesShaderPath.c_str()))
    {
        cout << vertexShader.log().toStdString() << " asd\n";
    }

    if (!fragmentShader.compileSourceFile(fragmentShaderPath.c_str()))
    {
        cout << fragmentShader.log().toStdString() << " asd\n";
    }

    if (!shaderProgramm_.addShader(&vertexShader))
    {
        cout << shaderProgramm_.log().toStdString() << " Shader compile programm problem\n";
    }

    if (!shaderProgramm_.addShader(&fragmentShader))
    {
        cout << shaderProgramm_.log().toStdString() << " Shader compile programm problem\n";
    }

    if (!shaderProgramm_.link())
    {
        cout << shaderProgramm_.log().toStdString() << " Shader compile programm problem\n";
    }
    return true;
}

bool GlSimulation::createShaderProgramFromFiles(QOpenGLShaderProgram &shaderProgramm_, std::string vertesShaderPath, std::string geometryShaderPath, std::string fragmentShaderPath)
{
    using namespace std;

    QOpenGLShaderProgram ret;

    QOpenGLShader vertexShader(QOpenGLShader::Vertex);
    QOpenGLShader geometryShader(QOpenGLShader::Geometry);
    QOpenGLShader fragmentShader(QOpenGLShader::Fragment);

    if (!vertexShader.compileSourceFile(vertesShaderPath.c_str()))
    {
        cout << vertexShader.log().toStdString() << " asd\n";
    }

    if (!geometryShader.compileSourceFile(geometryShaderPath.c_str()))
    {
        cout << geometryShader.log().toStdString() << " asd\n";
    }

    if (!fragmentShader.compileSourceFile(fragmentShaderPath.c_str()))
    {
        cout << fragmentShader.log().toStdString() << " asd\n";
    }

    if (!shaderProgramm_.addShader(&vertexShader))
    {
        cout << shaderProgramm_.log().toStdString() << " Shader compile programm problem\n";
    }

    if (!shaderProgramm_.addShader(&geometryShader))
    {
        cout << shaderProgramm_.log().toStdString() << " Shader compile programm problem\n";
    }

    if (!shaderProgramm_.addShader(&fragmentShader))
    {
        cout << shaderProgramm_.log().toStdString() << " Shader compile programm problem\n";
    }

    if (!shaderProgramm_.link())
    {
        cout << shaderProgramm_.log().toStdString() << " Shader compile programm problem\n";
    }
    return true;
}

void GlSimulation::readBodiesFromJson(std::string jsonName)
{
    pt ::ptree BodyList;

    try
    {
        read_json(jsonName, BodyList);
    }
    catch (pt ::json_parser_error &e)
    {
        std ::cout << "Failed to parse the json string.\n"
                   << e.what();
        throw;
    }
    catch (...)
    {
        std ::cout << "Failed !!!\n";
        throw;
    }

    auto BodiesIt = BodyList.find("bodies");

    for (auto &body : BodiesIt->second)
    {
        auto &elem = body.second;

        auto nameIt = elem.find("name");
        auto jsonPathIt = elem.find("jsonPath");

        if (
            nameIt == elem.not_found() || jsonPathIt == elem.not_found())
            throw;

        Body newElement(context(), jsonPathIt->second.data(), nameIt->second.data());
        _world->_bodies.emplace_back(std::move(newElement));
    }
}

void GlSimulation::readSettings(std::string jsonName)
{
    pt ::ptree settingsTree;

    try
    {
        read_json(jsonName, settingsTree);
    }
    catch (pt ::json_parser_error &e)
    {
        std ::cout << "Failed to parse the json string.\n"
                   << e.what();
        throw;
    }
    catch (...)
    {
        std ::cout << "Failed !!!\n";
        throw;
    }
    auto CamPosIt = settingsTree.find("CamPos");

    if (CamPosIt != settingsTree.not_found())
    {
        Eigen::Vector3d camPos;
        camPos.x() = std::next(CamPosIt->second.begin(), 0)->second.get_value<double>();
        camPos.y() = std::next(CamPosIt->second.begin(), 1)->second.get_value<double>();
        camPos.z() = std::next(CamPosIt->second.begin(), 2)->second.get_value<double>();
        _cam.setTranslationCam(camPos);
    }
}

void GlSimulation::resizeGL(int width, int height)
{
    _cam.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    _shadowMapTechPtr->resize(width, height);
    glViewport(0, 0, GLint(width), GLint(height));

    // std::cerr << "Resize. " << size().width() << " " << size().height() << "\n";
}