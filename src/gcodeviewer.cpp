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
		auto color = QColor::fromRgb(QRandomGenerator::global()->generate());
		QPen pen(color, 1, Qt::SolidLine);
		painter.setPen(pen);
		painter.drawPolygon(l);
		pen.setWidth(5);
		painter.setPen(pen);
		// painter.drawPoint(l[0]);
		// painter.drawPoint(l[l.size() - 1]);
	}
}

void GCodeViewer::setSlice(const std::vector<QPolygon> &slice) {
    lines.clear();
    // std::copy(slice.begin(), slice.end(), lines);
	lines = std::move(slice);

    for (auto &p : lines) {
        for (auto &pt : p) {
            pt /= 100000;
            pt += QPoint(400, 200);
        }
    }
}
