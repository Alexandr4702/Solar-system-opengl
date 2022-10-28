#include "glsimulation.h"

GlSimulation::GlSimulation(QWidget *parent): QGLWidget(parent)
{
    world = std::make_shared <World> ();
}

GlSimulation::~GlSimulation()
{

}

void GlSimulation::initializeGL()
{
    qglClearColor(Qt::black);
    glEnable(GL_DEPTH_TEST);
}

void GlSimulation::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    update();
}

void GlSimulation::resizeGL(int width, int height)
{
    glViewport(0, 0, GLint(width), GLint(height));
}
