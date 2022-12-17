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
#include <thread>
#include <chrono>
#include <unordered_map>

#include "world.h"
#include "camera.h"

class GlSimulation : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT
public:
    GlSimulation(QWidget *parent);
    ~GlSimulation();

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void paintThreadfoo();
    void initShader();
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    void drawThread();
    // void drawCube();
    bool createShaderProgramFromFiles(QOpenGLShaderProgram &shaderProgramm_, std::string vertesShader, std::string fragmentShader);
    QOpenGLShaderProgram _shaderProgrammBody;
    QOpenGLShaderProgram _shaderProgrammShadowMap;
    std::shared_ptr<World> _world;
    Camera _cam;
    std::thread _paintThreadHandle;
    std::atomic<bool> _isPaintThreadRun = true;

    std::unordered_map<int, bool> _PressedKey;
    std::mutex _pressedKeyMutex;
};

#endif // GLSIMULATION_H
