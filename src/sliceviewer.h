#ifndef SLICEVIEWER_H
#define SLICEVIEWER_H

#include "model3d.h"
#include <QDockWidget>

class SliceWidget;
class QCheckBox;
class QLabel;

/**
 * Viewer for sliced paths.
 */
class SliceViewer : public QDockWidget {
    Q_OBJECT
  public:
    explicit SliceViewer(QWidget *parent = nullptr);
	void setSlices(std::vector<std::vector<QPolygon>> slices);

  private:
	QCheckBox *randColorBtn;
	QLabel *layerLbl, *layerTotalLbl;
	std::vector<std::vector<QPolygon>> slices;
	std::size_t currentSlice;
	SliceWidget *slice;

  public slots:
	void nextSlice();
	void prevSlice();
	void randColorToggle(bool value);
	void changeColor();
	void play();
	void reset();
};

#endif // SLICEVIEWER_H
