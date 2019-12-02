#include "sliceviewer.h"
#include "slicewidget.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QRandomGenerator>

SliceViewer::SliceViewer(QWidget *parent) : QDockWidget(parent) {
    currentSlice = 0;
    slice = new SliceWidget(this);
	setWidget(slice);
}

void SliceViewer::setSlices(std::vector<std::vector<QPolygon>> slices) {
    this->slices = slices;
    currentSlice = 0;
    if (this->slices.size() > 0) {
        slice->setSlice(this->slices[currentSlice]);
    }
}
