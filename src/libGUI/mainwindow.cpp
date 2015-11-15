#include <iostream>

#include <QMdiSubWindow>
#include <QMenuBar>
#include <QObject>
#include <QRect>
#include <QSize>

#include "mainwindow.h"
#include "viewwidget.h"
#include "settings.h"

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags) :
    QMainWindow(parent, flags),
    _desktop(0),
    _mdiArea(new QMdiArea(this)),
    _tabletActive(false),
    _rootScene(new RootScene())
{
    QMenuBar* menuBar = this->menuBar();
    QMenu* menuTest = menuBar->addMenu("Test");

    menuTest->addAction("Add scene viewer", this, SLOT(onCreateViewer()));
    menuTest->addAction("Add scene double viewer", this, SLOT(onCreateDoubleViewer()));
    menuTest->addAction("Add outisde viewer", this, SLOT(onCreateOutsideViewer()));
    menuTest->addAction("Load cow to the scene",  this, SLOT(onLoadCow()));
    menuTest->addAction("Stylus draw ON", this, SLOT(onSetStylusSketchON()));
    menuTest->addAction("Stylus draw OFF", this, SLOT(onSetStylusSketchOFF()));
    menuTest->addAction("Global axes ON", this, SLOT(onSetGloAxesON()));
    menuTest->addAction("Global axes OFF", this, SLOT(onSetGloAxesOFF()));

    this->setCentralWidget(_mdiArea);
}

MainWindow::~MainWindow(){}

void MainWindow::SetDesktopWidget(QDesktopWidget *desktop, dureu::APPMODE mode) {
    _desktop = desktop;
    QRect availS = _desktop->availableGeometry();
    QRect fullS = _desktop->geometry();
    int nscreen = _desktop->screenCount();
    double scale = 0.9;
    double scale_inv = 1-scale;
    switch (mode) {
    case dureu::APPMODE::MIN_SCREEN:
        this->resize(QSize(availS.width()*scale, availS.height()*scale));
        this->move(availS.width()*scale_inv, fullS.height()-availS.height());
        break;
    case dureu::APPMODE::MAX_SCREEN:
        this->showMaximized();
        break;
    case dureu::APPMODE::FULL_SCREEN:
        this->showFullScreen();
        break;
    case dureu::APPMODE::VIRTUAL_WINDOW: // needs testing and fixing
        this->resize(QSize(fullS.width(), fullS.height()));
        break;
    case dureu::APPMODE::DISATTACHED_MENU:
        this->resize(QSize(availS.width()*scale, fullS.height()*scale_inv));
        this->move(availS.width()*scale_inv, fullS.height()-availS.height());
        break;
    default:
        std::cerr << "Application mode not recognized, closing application" << std::endl;
        exit(1);
    }
}

void MainWindow::getTabletActivity(bool active){
    _tabletActive = active;
    emit sendTabletActivity(active);
}

/* Create an ordinary single view window on the scene _root */
void MainWindow::onCreateViewer(){    
    ViewWidget* vwid = new ViewWidget(_rootScene, this);
    QObject::connect(this, SIGNAL(sendTabletActivity(bool)),
                     vwid, SLOT(getTabletActivity(bool)));
    QObject::connect(this, SIGNAL(sendStylusSketchStatus(bool)),
                     vwid, SLOT(getStylusSketchStatus(bool)) );
    QMdiSubWindow* subwin = _mdiArea->addSubWindow(vwid);
    subwin->show();
}

void MainWindow::onCreateDoubleViewer(){
    ViewWidget* vwid = new ViewWidget(_rootScene, this, Qt::Widget, 2);
    QObject::connect(this, SIGNAL(sendTabletActivity(bool)),
                     vwid, SLOT(getTabletActivity(bool)));
    QObject::connect(this, SIGNAL(sendStylusSketchStatus(bool)),
                     vwid, SLOT(getStylusSketchStatus(bool)) );
    QMdiSubWindow* subwin = _mdiArea->addSubWindow(vwid);
    subwin->show();
}

void MainWindow::onCreateOutsideViewer(){
    ViewWidget* vwid = new ViewWidget(_rootScene, this, Qt::Window);
    QObject::connect(this, SIGNAL(sendTabletActivity(bool)),
                     vwid, SLOT(getTabletActivity(bool)));
    QObject::connect(this, SIGNAL(sendStylusSketchStatus(bool)),
                     vwid, SLOT(getStylusSketchStatus(bool)) );
    vwid->show();
}

void MainWindow::onLoadCow(){
    bool success = _rootScene->loadSceneFromFile("../../samples/cow.osgt");
    if (!success){
        std::cerr << "The filename provided was not correct, or the model could not be read" << std::endl;
    }
}

void MainWindow::onSetStylusSketchON(){
    emit sendStylusSketchStatus(true);
}

void MainWindow::onSetStylusSketchOFF(){
    emit sendStylusSketchStatus(false);
}

void MainWindow::onSetGloAxesON() {
    _rootScene->setAxesVisible();
}

void MainWindow::onSetGloAxesOFF() {
    _rootScene->setAxesInvisible();
}

