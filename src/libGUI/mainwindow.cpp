#include <iostream>

#include <QMdiSubWindow>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QtWidgets>
#include <QObject>
#include <QRect>
#include <QSize>
#include <QDebug>

#include "mainwindow.h"
#include "glwidget.h"
#include "settings.h"
#include "Data.h"

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , m_desktop(0)
    , m_mdiArea(new QMdiArea(this))
    , m_undoStack(new QUndoStack(this))
    , m_undoView(new QUndoView(m_undoStack))
    , m_menuBar(new QMenuBar(0)) // http://stackoverflow.com/questions/8108729/qmenu-does-not-work-on-mac-qt-creator
    , m_rootScene(new RootScene(m_undoStack))
{

    this->setMenuBar(m_menuBar);
    this->onCreateViewer();

    m_undoView->setWindowTitle(tr("Command List"));
    m_undoView->show();
    m_undoView->setAttribute(Qt::WA_QuitOnClose, false);

    this->setCentralWidget(m_mdiArea);
    this->initializeActions();
}

MainWindow::~MainWindow(){
    if (m_menuBar)
        delete m_menuBar;
}

void MainWindow::SetDesktopWidget(QDesktopWidget *desktop, dureu::APPMODE mode) {
    m_desktop = desktop;
    QRect availS = m_desktop->availableGeometry();
    QRect fullS = m_desktop->geometry();
    //int nscreen = m_desktop->screenCount();
    double scale = 0.9;
    double scale_inv = 1-scale;
    switch (mode) {
    case dureu::SCREEN_MIN:
        this->showNormal();
        break;
    case dureu::SCREEN_MAX:
        this->showMaximized();
        break;
    case dureu::SCREEN_FULL:
        this->showFullScreen();
        break;
    case dureu::SCREEN_VIRTUAL: // needs testing and fixing
        this->resize(QSize(fullS.width(), fullS.height()));
        break;
    case dureu::SCREEN_DETACHED:
        this->resize(QSize(availS.width()*scale, fullS.height()*scale_inv));
        this->move(availS.width()*scale_inv, fullS.height()-availS.height());
        break;
    default:
        std::cerr << "Application mode not recognized, closing application" << std::endl;
        exit(1);
    }
    std::cout << "Widget width and height: " << this->width() << " " << this->height() << std::endl;
}

void MainWindow::getTabletActivity(bool active){
    emit sendTabletActivity(active);
}

/* Create an ordinary single view window on the scene _root
 * To create outside viewer, use:
 * GLWidget* vwid = createViewer(Qt::Window);
*/
void MainWindow::onCreateViewer(){
    GLWidget* vwid = createViewer();
    QMdiSubWindow* subwin = m_mdiArea->addSubWindow(vwid);
    subwin->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    vwid->showMaximized();
    subwin->show();
}

void MainWindow::onMouseOrbit(){
    emit sendMouseMode(dureu::MOUSE_ROTATE);
}

void MainWindow::onMouseZoom(){
    emit sendMouseMode(dureu::MOUSE_ZOOM);
}

void MainWindow::onMousePan(){
    emit sendMouseMode(dureu::MOUSE_PAN);
}

void MainWindow::onMousePick(){
    emit sendMouseMode(dureu::MOUSE_PICK);
}

void MainWindow::onMouseErase()
{
    emit sendMouseMode(dureu::MOUSE_ERASE);
}

void MainWindow::onMouseDelete()
{
    emit sendMouseMode(dureu::MOUSE_DELETE);
}

void MainWindow::onMouseSketch()
{
    emit sendMouseMode(dureu::MOUSE_SKETCH);
}

void MainWindow::onMouseOffset()
{
    emit sendMouseMode(dureu::MOUSE_EDIT_OFFSET);
}

void MainWindow::onMouseRotate()
{
    emit sendMouseMode(dureu::MOUSE_EDIT_ROTATE);
}

void MainWindow::onMouseMove()
{
    emit sendMouseMode(dureu::MOUSE_EDIT_MOVE);
}

void MainWindow::onFileImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load an Image File"), QString(),
            tr("Image Files (*.bmp)"));
    if (!fileName.isEmpty()) {
        if (!m_rootScene->loadPhotoFromFile(fileName.toStdString())){
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
    }
}

GLWidget* MainWindow::createViewer(Qt::WindowFlags f, int viewmode)
{
    GLWidget* vwid = new GLWidget(m_rootScene.get(), this, f);
    QObject::connect(this, SIGNAL(sendTabletActivity(bool)),
                     vwid, SLOT(getTabletActivity(bool)));
    QObject::connect(this, SIGNAL(sendMouseMode(dureu::MOUSE_MODE)),
                     vwid, SLOT(recieveMouseMode(dureu::MOUSE_MODE)));
    return vwid;
}

void MainWindow::initializeActions()
{
    // FILE
    m_actionNewFile = new QAction(Data::fileNewSceneIcon(), tr("&New..."), this);

    m_actionClose = new QAction(Data::fileCloseIcon(), tr("&Close"), this);

    m_actionExit = new QAction(Data::fileExitIcon(), tr("&Exit"), this);

    m_actionImportImage = new QAction(Data::fileExitIcon(), tr("Import &Image..."), this);
    connect(m_actionImportImage, SIGNAL(triggered(bool)), this, SLOT(onFileImage()));

    m_actionOpenFile = new QAction(Data::fileOpenIcon(), tr("&Open..."), this);

    m_actionSaveFile = new QAction(Data::fileSaveIcon(), tr("&Save..."), this);

    // EDIT

    m_actionUndo = m_undoStack->createUndoAction(this, tr("&Undo"));
    m_actionUndo->setIcon(Data::editUndoIcon());
    m_actionUndo->setShortcuts(QKeySequence::Undo);

    m_actionRedo = m_undoStack->createRedoAction(this, tr("&Redo"));
    m_actionRedo->setIcon(Data::editRedoIcon());
    m_actionRedo->setShortcuts(QKeySequence::Redo);

    m_actionCut = new QAction(Data::editCutIcon(), tr("&Cut"), this);

    m_actionCopy = new QAction(Data::editCopyIcon(), tr("C&opy"), this);

    m_actionPaste = new QAction(Data::editPasteIcon(), tr("&Paste"), this);

    m_actionDelete = new QAction(Data::editDeleteIcon(), tr("&Delete"), this);
    this->connect(m_actionDelete, SIGNAL(triggered(bool)), this, SLOT(onMouseDelete()));

    // SCENE

    m_actionSketch = new QAction(Data::sceneSketchIcon(), tr("&Sketch"), this);
    this->connect(m_actionSketch, SIGNAL(triggered(bool)), this, SLOT(onMouseSketch()));

    m_actionEraser = new QAction(Data::sceneEraserIcon(), tr("&Eraser"), this);
    this->connect(m_actionEraser, SIGNAL(triggered(bool)), this, SLOT(onMouseErase()));

    m_actionSelect = new QAction(tr("S&elect"), this);
    this->connect(m_actionSelect, SIGNAL(triggered(bool)), this, SLOT(onMousePick()));

    m_actionCanvasClone = new QAction(Data::sceneNewCanvasCloneIcon(), tr("Clone Current"), this);

    m_actionCanvasXY = new QAction(Data::sceneNewCanvasXYIcon(), tr("Plane XY"), this);

    m_actionCanvasYZ = new QAction(Data::sceneNewCanvasYZIcon(), tr("Plane YZ"), this);

    m_actionCanvasXZ = new QAction(Data::sceneNewCanvasXZIcon(), tr("Plane XZ"), this);

    m_actionCanvasOffset = new QAction(Data::sceneCanvasOffsetIcon(), tr("Offset Canvas"), this);
    this->connect(m_actionCanvasOffset, SIGNAL(triggered(bool)), this, SLOT(onMouseOffset()));

    m_actionCanvasRotate = new QAction(Data::sceneCanvasRotateIcon(), tr("Rotate Canvas"), this);
    this->connect(m_actionCanvasRotate, SIGNAL(triggered(bool)), this, SLOT(onMouseRotate()));

    m_actionImageMove = new QAction(Data::sceneImageMoveIcon(), tr("Move Image"), this);
    this->connect(m_actionImageMove, SIGNAL(triggered(bool)), this, SLOT(onMouseMove()));

    m_actionImageRotate = new QAction(Data::sceneImageRotateIcon(), tr("Rotate Image"), this);
    this->connect(m_actionImageRotate, SIGNAL(triggered(bool)), this, SLOT(onMouseRotateImage()));

    m_actionImageScale = new QAction(Data::sceneImageScaleIcon(), tr("Scale Image"), this);

    m_actionImageFlip = new QAction(Data::sceneImageFlipIcon(), tr("Flip Image"), this);

    m_actionImagePush = new QAction(Data::sceneImagePushIcon(), tr("Push Image"), this);

    m_actionStrokesPush = new QAction(Data::scenePushStrokesIcon(), tr("Push Strokes"), this);
    this->connect(m_actionStrokesPush, SIGNAL(triggered(bool)), this, SLOT(onMousePushStrokes()));
}

/*_mActionOrbit = new QAction(QIcon(":/orbit.png"),tr("&orbit"), this);
    _mActionOrbit->setStatusTip(tr("orbit"));
    this->connect(_mActionOrbit, SIGNAL(triggered()), this, SLOT(onMouseOrbit()));

    _mActionPan = new QAction(QIcon(":/pan.png"),tr("&Pan"), this);
    _mActionPan->setStatusTip(tr("Pan"));
    this->connect(_mActionPan, SIGNAL(triggered()), this, SLOT(onMousePan()));

    _mActionZoom = new QAction(QIcon(":/zoom.png"),tr("&Zoom"), this);
    _mActionZoom->setStatusTip(tr("Zoom"));
    this->connect(_mActionZoom, SIGNAL(triggered()), this, SLOT(onMouseZoom()));*/
