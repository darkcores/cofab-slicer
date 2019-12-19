#include "mainwindow.h"
#include "gcodegenerator.h"
#include "model3d.h"
#include "sliceprocessor.h"
#include "sliceroptions.h"
#include "sliceviewer.h"
#include "stlviewer.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Fix layout without central widget
    this->setDockNestingEnabled(true);
    this->resize(1200, 800);
    model = nullptr;

    setupMenu();
    setupDocks();
}

MainWindow::~MainWindow() {
    if (model)
        delete model;
}

void MainWindow::setupMenu() {
    // File menu & actions
    auto menu = menuBar()->addMenu(tr("&File"));
    openAct = menu->addAction(tr("&Open file"));
    openAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);
    exportAct = menu->addAction(tr("&Export"));
    exportAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    connect(exportAct, &QAction::triggered, this, &MainWindow::exportGcode);
    quitAct = menu->addAction(tr("&Quit"));
    quitAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    connect(quitAct, &QAction::triggered, this, &MainWindow::quit);

    // View menu & action (toggle visibility)
    auto view = menuBar()->addMenu(tr("&View"));
    optionViewAct = view->addAction(tr("&Options dock"));
    optionViewAct->setCheckable(true);
    stlViewAct = view->addAction(tr("&STL view dock"));
    stlViewAct->setCheckable(true);
    gcodeViewAct = view->addAction(tr("&Gcode view dock"));
    gcodeViewAct->setCheckable(true);
}

void MainWindow::setupDocks() {
    slicerOptions = new SlicerOptions(this);
    addDockWidget(Qt::LeftDockWidgetArea, slicerOptions);
    stlViewer = new STLViewer(this);
    addDockWidget(Qt::RightDockWidgetArea, stlViewer);
    gcodeViewer = new SliceViewer(this);
    addDockWidget(Qt::RightDockWidgetArea, gcodeViewer);
}

void MainWindow::openFile() {
    QUrl file = QFileDialog::getOpenFileUrl(this, tr("Open 3D model"), QUrl(),
                                            tr("3D model (*.stl)"));
    loadFile(file);
}

void MainWindow::loadFile(QUrl file) {
    stlViewer->loadStl(file);
    if (model)
        delete model;
    model = new Model3D(file.path().toStdString());
    auto slices = model->getSlices();
    auto bounds = model->getBounds();
    // gcodeViewer->setSlice(slice);
    SliceProcessor sp(bounds);
    auto clipped = sp.process(slices);
    gcodeViewer->setSlices(clipped);

    // move to center of the bed
    QPoint offset((110 * 1000) - (bounds.width() / 2),
                  (110 * 1000) - (bounds.height() / 2));
    for (auto &layer : clipped) {
        for (auto &path : layer) {
            for (auto &point : path) {
                point += offset;
            }
        }
    }

	GCodeGenerator g("test.gcode");
    // g.generateGcode(clipped, "test.gcode");
    g.exportSlices(clipped);
}

void MainWindow::exportGcode() {}

void MainWindow::quit() { QApplication::quit(); }
