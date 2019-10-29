#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class SlicerOptions;
class STLViewer;
class GCodeViewer;
class QAction;

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  private:
	SlicerOptions *slicerOptions;
	STLViewer *stlViewer;
	GCodeViewer *gcodeViewer;
	QAction *openAct, *exportAct, *quitAct;
	QAction *optionViewAct, *stlViewAct, *gcodeViewAct;

    void setupMenu();
	void setupDocks();

  private slots:
    void openFile();
    void exportGcode();
    void quit();
};
#endif // MAINWINDOW_H
