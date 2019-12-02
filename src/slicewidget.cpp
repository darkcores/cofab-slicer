#include "slicewidget.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QRandomGenerator>

#include <iostream>

SliceWidget::SliceWidget(QWidget *parent) : QWidget(parent) {
	random_color = true;
}

void SliceWidget::paintEvent(QPaintEvent *event) {
    (void)event; // To ignore compiler warning unused.
    QPainter painter(this);
    // painter.setRenderHint(QPainter::Antialiasing, true);
    for (auto &l : lines) {
        if (random_color)
            color = QColor::fromRgb(QRandomGenerator::global()->generate());
        QPen pen(color, 1, Qt::SolidLine);
        painter.setPen(pen);
        painter.drawPolygon(l);
        pen.setWidth(5);
        painter.setPen(pen);
    }
}

void SliceWidget::setSlice(const std::vector<QPolygon> slice) {
    lines = slice;
	for (auto &line : lines) {
		for (auto &pt : line) {
            pt /= 100000;
        }
	}
    this->repaint();
}

void SliceWidget::setColor(const QColor color) {
    random_color = false;
    this->color = color;
	this->repaint();
}

void SliceWidget::setRandomColor(const bool random) {
	random_color = random;
	this->repaint();
}
