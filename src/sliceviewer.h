#ifndef SLICEVIEWER_H
#define SLICEVIEWER_H

#include "model3d.h"
#include <QDockWidget>

class SliceWidget;

/**
 * Viewer for sliced paths.
 */
class SliceViewer : public QDockWidget {
    Q_OBJECT
  public:
    explicit SliceViewer(QWidget *parent = nullptr);
	void setSlices(std::vector<std::vector<QPolygon>> slices);

  private:
	std::vector<std::vector<QPolygon>> slices;
	std::size_t currentSlice;
	SliceWidget *slice;

  public slots:
	// void nextSlice();
	// void prevSlice();
};

#endif // SLICEVIEWER_H
