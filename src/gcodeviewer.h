#ifndef GCODEVIEWER_H
#define GCODEVIEWER_H

#include <QDockWidget>
#include "model3d.h"

class QPaintEvent;

class GCodeViewer : public QDockWidget {
    Q_OBJECT
  public:
    explicit GCodeViewer(QWidget *parent = nullptr);

	void setSlice(const std::vector<Model3D::Line> &slice);

  protected:
    void paintEvent(QPaintEvent *event);

private:
	std::vector<Model3D::Line> lines;

  signals:

  public slots:
};

#endif // GCODEVIEWER_H
