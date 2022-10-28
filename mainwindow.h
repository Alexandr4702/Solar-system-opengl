#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "glsimulation.h"
#include <memory>

class Mainwindow:public QWidget
{
public:
    Mainwindow();
    float simulationScreenRatio = 0.7;
    std::shared_ptr <GlSimulation> simulation;
protected:
   void resizeEvent(QResizeEvent *event) override;
};

#endif // MAINWINDOW_H
