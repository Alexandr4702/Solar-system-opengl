#include "mainwindow.h"

Mainwindow::Mainwindow()
{
    resize(800, 600);
    _simulation = std::make_shared<GlSimulation>(this);
    _simulation->setGeometry(QRect(0, 0, 800 * _simulationScreenRatio, 600));
}

void Mainwindow::resizeEvent(QResizeEvent *event)
{
    _simulation->setGeometry(QRect(0, 0, event->size().width() * _simulationScreenRatio, event->size().height()));
}
