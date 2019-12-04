#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class SlicerOptions;
class STLViewer;
class SliceViewer;
class QAction;
class Model3D;

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadFile(QUrl filename);

  private:
    SlicerOptions *slicerOptions;
    STLViewer *stlViewer;
    SliceViewer *gcodeViewer;
    QAction *openAct, *exportAct, *quitAct;
    QAction *optionViewAct, *stlViewAct, *gcodeViewAct;
    Model3D *model;

    void setupMenu();
    void setupDocks();

  private slots:
    void openFile();
    void exportGcode();
    void quit();
};
#endif // MAINWINDOW_H
