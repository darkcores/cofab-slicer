#include "gcodeviewer.h"

#include <QPaintEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QPen>

GCodeViewer::GCodeViewer(QWidget *parent) : QDockWidget(parent) {}

void GCodeViewer::paintEvent(QPaintEvent *event) {
    (void)event; // To ignore compiler warning unused.
    QPainter painter(this);
    // painter.setRenderHint(QPainter::Antialiasing, true);

    // painter.drawRect(0, 0, 100, 100);
	for (auto &l : lines) {
		QPen pen(QColor::fromRgb(QRandomGenerator::global()->generate()), 2, Qt::SolidLine);
		painter.setPen(pen);
		painter.drawPolygon(l);
	}
}

void GCodeViewer::setSlice(const std::vector<QPolygonF> &slice) {
    lines.clear();
    // std::copy(slice.begin(), slice.end(), lines);
	lines = std::move(slice);
}
