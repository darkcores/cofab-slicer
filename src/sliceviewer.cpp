#include "sliceviewer.h"
#include "slicewidget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QRandomGenerator>
#include <QColorDialog>

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
    if (this->slices.size() > 0) {
        slice->setSlice(this->slices[currentSlice]);
    }
}

void SliceViewer::nextSlice() {
    if (currentSlice + 1 < slices.size()) {
        currentSlice++;
        slice->setSlice(this->slices[currentSlice]);
    }
}

void SliceViewer::prevSlice() {
    if (currentSlice > 0) {
        currentSlice--;
        slice->setSlice(this->slices[currentSlice]);
    }
}

void SliceViewer::randColorToggle(bool value) { slice->setRandomColor(value); }

void SliceViewer::changeColor() {
	randColorBtn->setChecked(false);
    auto newColor = QColorDialog::getColor(slice->getColor(), this);
	slice->setColor(newColor);
}
