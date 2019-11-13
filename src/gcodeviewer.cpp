#include "gcodeviewer.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPen>

GCodeViewer::GCodeViewer(QWidget *parent) : QDockWidget(parent) {}

void GCodeViewer::paintEvent(QPaintEvent *event) {
    (void)event; // To ignore compiler warning unused.
    QPainter painter(this);
    QPen pen(Qt::black, 2, Qt::SolidLine);
    painter.setPen(pen);
    // painter.setRenderHint(QPainter::Antialiasing, true);

    // painter.drawRect(0, 0, 100, 100);
	for (auto &l : lines) {
		painter.drawLine(l);
	}
}

void GCodeViewer::setSlice(const std::vector<QLineF> &slice) {
    lines.clear();
    // std::copy(slice.begin(), slice.end(), lines);
	lines = std::move(slice);
}
