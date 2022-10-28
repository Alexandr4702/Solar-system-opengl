#include "mainwindow.h"

Mainwindow::Mainwindow()
{
    resize(800, 600);
    simulation = std::make_shared<GlSimulation> (this);
    simulation->setGeometry(QRect(0, 0, 800 * simulationScreenRatio, 600));
}

void Mainwindow::resizeEvent(QResizeEvent *event)
{
    simulation->setGeometry(QRect(0, 0, event->size().width() * simulationScreenRatio, event->size().height()));
}
