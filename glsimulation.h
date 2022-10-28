#ifndef GLSIMULATION_H
#define GLSIMULATION_H
#include <QGLWidget>
#include <QtOpenGL>

#include "world.h"

class GlSimulation: public QGLWidget
{
public:
    GlSimulation(QWidget *parent);
    ~GlSimulation();
protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void initShader();
private:
    std::shared_ptr <World> world;
};

#endif // GLSIMULATION_H
