#include "slicewidget.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QRandomGenerator>

SliceWidget::SliceWidget(QWidget *parent) : QWidget(parent) {}

void SliceWidget::paintEvent(QPaintEvent *event) {
    (void)event; // To ignore compiler warning unused.
    QPainter painter(this);
    // painter.setRenderHint(QPainter::Antialiasing, true);
    for (auto &l : *lines) {
        auto color = QColor::fromRgb(QRandomGenerator::global()->generate());
        QPen pen(color, 1, Qt::SolidLine);
        painter.setPen(pen);
        painter.drawPolygon(l);
        pen.setWidth(5);
        painter.setPen(pen);
    }
}

void SliceWidget::setSlice(const std::vector<QPolygon> *slice) {
    lines = slice;
	this->repaint();
}
