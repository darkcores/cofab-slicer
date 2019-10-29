#ifndef GCODEVIEWER_H
#define GCODEVIEWER_H

#include <QDockWidget>

class GCodeViewer : public QDockWidget {
    Q_OBJECT
  public:
    explicit GCodeViewer(QWidget *parent = nullptr);

  signals:

  public slots:
};

#endif // GCODEVIEWER_H
