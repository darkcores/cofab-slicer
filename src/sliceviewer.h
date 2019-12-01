#ifndef SLICEVIEWER_H
#define SLICEVIEWER_H

#include "model3d.h"
#include <QDockWidget>

class SliceViewer : public QDockWidget {
    Q_OBJECT
  public:
    explicit SliceViewer(QWidget *parent = nullptr);
	void setSlices(const std::vector<std::vector<QPolygon>> *slices);

  private:
	const std::vector<std::vector<QPolygon>> *slices;
	std::size_t currentSlice;

  public slots:
	// void nextSlice();
	// void prevSlice();
};

#endif // SLICEVIEWER_H
