#include "sliceviewer.h"
#include "slicewidget.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QRandomGenerator>
#include <QTimer>

SliceViewer::SliceViewer(QWidget *parent) : QDockWidget(parent) {
    currentSlice = 0;

    auto layout = new QVBoxLayout();
    slice = new SliceWidget(this);
    slice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(slice);

    auto controls = new QHBoxLayout();

    randColorBtn = new QCheckBox("Random colors");
    randColorBtn->setChecked(true);
    connect(randColorBtn, &QCheckBox::toggled, this,
            &SliceViewer::randColorToggle);
    controls->addWidget(randColorBtn);

    auto colorBtn = new QPushButton("color");
    connect(colorBtn, &QPushButton::clicked, this, &SliceViewer::changeColor);
    controls->addWidget(colorBtn);

    auto prevBtn = new QPushButton("Previous");
    connect(prevBtn, &QPushButton::clicked, this, &SliceViewer::prevSlice);
    controls->addWidget(prevBtn);

    auto nextBtn = new QPushButton("Next");
    connect(nextBtn, &QPushButton::clicked, this, &SliceViewer::nextSlice);
    controls->addWidget(nextBtn);

    auto zoomBtn = new QPushButton("Zoom In");
    connect(zoomBtn, &QPushButton::clicked, slice, &SliceWidget::zoomIn);
    controls->addWidget(zoomBtn);

    auto zoomOutBtn = new QPushButton("Zoom Out");
    connect(zoomOutBtn, &QPushButton::clicked, slice, &SliceWidget::zoomOut);
    controls->addWidget(zoomOutBtn);

    auto playBtn = new QPushButton("Play");
    connect(playBtn, &QPushButton::clicked, this, &SliceViewer::play);
    controls->addWidget(playBtn);

    auto resetBtn = new QPushButton("Reset");
    connect(resetBtn, &QPushButton::clicked, this, &SliceViewer::reset);
    controls->addWidget(resetBtn);

    layerLbl = new QLabel(QString::number(currentSlice));
	controls->QLayout::addWidget(layerLbl);

	layerTotalLbl = new QLabel("/ 0");
	controls->addWidget(layerTotalLbl);

    auto controlWidget = new QWidget(this);
    controlWidget->setLayout(controls);
    layout->addWidget(controlWidget);

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(layout);
    setWidget(mainWidget);
}

void SliceViewer::setSlices(std::vector<std::vector<QPolygon>> slices) {
    this->slices = slices;
    currentSlice = 0;
    layerLbl->setText(QString::number(currentSlice));
	layerTotalLbl->setText(QString("/ ") + QString::number(slices.size()));
    if (this->slices.size() > 0) {
        slice->setSlice(this->slices[currentSlice]);
    }
}

void SliceViewer::nextSlice() {
    if (currentSlice + 1 < slices.size()) {
        currentSlice++;
        layerLbl->setText(QString::number(currentSlice));
        slice->setSlice(this->slices[currentSlice]);
    }
}

void SliceViewer::prevSlice() {
    if (currentSlice > 0) {
        currentSlice--;
        layerLbl->setText(QString::number(currentSlice));
        slice->setSlice(this->slices[currentSlice]);
    }
}

void SliceViewer::randColorToggle(bool value) { slice->setRandomColor(value); }

void SliceViewer::changeColor() {
    randColorBtn->setChecked(false);
    auto newColor = QColorDialog::getColor(slice->getColor(), this);
    slice->setColor(newColor);
}

void SliceViewer::play() {
    if (currentSlice + 1 < slices.size()) {
        currentSlice++;
        layerLbl->setText(QString::number(currentSlice));
        slice->setSlice(this->slices[currentSlice]);
        QTimer::singleShot(175, this, SLOT(play()));
    }
}

void SliceViewer::reset() {
    currentSlice = 0;
    slice->setSlice(this->slices[currentSlice]);
    layerLbl->setText(QString::number(currentSlice));
}
