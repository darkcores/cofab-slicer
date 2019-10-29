#include "mainwindow.h"
#include "sliceroptions.h"
#include "stlviewer.h"
#include "gcodeviewer.h"

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QMenuBar>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Fix layout without central widget
    this->setDockNestingEnabled(true);

    setupMenu();
	setupDocks();
}

MainWindow::~MainWindow() {}

void MainWindow::setupMenu() {
	// File menu & actions
    auto menu = menuBar()->addMenu(tr("&File"));
    openAct = menu->addAction(tr("&Open file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);
    exportAct = menu->addAction(tr("&Export"));
    connect(exportAct, &QAction::triggered, this, &MainWindow::exportGcode);
    quitAct = menu->addAction(tr("&Quit"));
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
	gcodeViewer = new GCodeViewer(this);
	addDockWidget(Qt::BottomDockWidgetArea, gcodeViewer);
}

void MainWindow::openFile() {}

void MainWindow::exportGcode() {}

void MainWindow::quit() { QApplication::quit(); }
