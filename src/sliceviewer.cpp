#include "sliceviewer.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QRandomGenerator>

SliceViewer::SliceViewer(QWidget *parent) : QDockWidget(parent) {
    currentSlice = 0;
}

void SliceViewer::setSlices(const std::vector<std::vector<QPolygon>> *slices) {
    this->slices = slices;
}
