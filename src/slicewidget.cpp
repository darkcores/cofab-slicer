#include "slicewidget.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QRandomGenerator>

#include <iostream>

SliceWidget::SliceWidget(QWidget *parent) : QWidget(parent) {
    random_color = true;
	lastpoint = QPoint(0, 0);
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
	original = slice;
    lines = slice;
    for (auto &line : lines) {
        for (auto &pt : line) {
			// pt -= QPoint(110 * 10000, 110 * 10000);
            pt /= 150;
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

void SliceWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
		lastpoint = QPoint(event->x(), event->y());
        mouse_down = true;
    }
}

void SliceWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        mouse_down = false;
    }
}

void SliceWidget::mouseMoveEvent(QMouseEvent *event) {
    if (mouse_down) {
		auto point = QPoint(event->x(), event->y());
        for (auto &line : lines) {
			for (auto &p: line) {
				p += (point - lastpoint);
			}
        }
		lastpoint = point;
		this->repaint();
    }
}

void SliceWidget::wheelEvent(QWheelEvent *event) {
	// std::cout << "Delta: " << event->angleDelta().y() << " "; 
	/*
	float scale = 1.0f + (event->angleDelta().y() / 256.0f);
	std::cout << "Scale: " << scale << std::endl;
	for (auto &line : lines) {
		for (auto &p: line) {
			p *= scale;
		}
	}
	this->repaint();
	*/
}

void SliceWidget::zoomIn() {
	if (zoomlvl < 12) {
		lines = original;
		zoomlvl ++;
		for (auto &line : lines) {
			for (auto &p: line) {
				p *= ((1.0f + (zoomlvl / 5.0f)) / 150);
				p += lastpoint;
			}
		}
		this->repaint();
	}
}

void SliceWidget::zoomOut() {
	if (zoomlvl > -12) {
		lines = original;
		zoomlvl --;
		for (auto &line : lines) {
			for (auto &p: line) {
				p *= ((1.0f + (zoomlvl / 5.0f)) / 150);
				p += lastpoint;
			}
		}
		this->repaint();
	}
}
