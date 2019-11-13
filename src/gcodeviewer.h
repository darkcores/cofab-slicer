#ifndef GCODEVIEWER_H
#define GCODEVIEWER_H

#include <QDockWidget>
#include "model3d.h"

class QPaintEvent;

class GCodeViewer : public QDockWidget {
    Q_OBJECT
  public:
    explicit GCodeViewer(QWidget *parent = nullptr);

	void setSlice(const std::vector<QLineF> &slice);

  protected:
    void paintEvent(QPaintEvent *event);

private:
	std::vector<QLineF> lines;

  signals:

  public slots:
};

#endif // GCODEVIEWER_H
