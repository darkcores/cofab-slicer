#ifndef STLVIEWER_H
#define STLVIEWER_H

#include <QDockWidget>

class STLViewer : public QDockWidget {
    Q_OBJECT
  public:
    explicit STLViewer(QWidget *parent = nullptr);

  signals:

  public slots:
};

#endif // STLVIEWER_H
