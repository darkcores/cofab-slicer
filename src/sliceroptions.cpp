#include "sliceroptions.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolBox>

SlicerOptions::SlicerOptions(QWidget *parent) : QDockWidget(parent) {
	auto toolbox = new QToolBox(this);
	
	auto slc = slicerSettings();
    toolbox->addItem(slc, tr("Slicer Settings"));
	auto obj = objectSettings();
    toolbox->addItem(obj, tr("Object Settings"));
	auto prnt = printerSettings();
    toolbox->addItem(prnt, tr("Printer Settings"));

	setWidget(toolbox);
}

QWidget *SlicerOptions::slicerSettings() {
    auto w = new QWidget(this);
    auto layout = new QFormLayout(w);

    layout->addRow(tr("Retraction"), new QCheckBox(this));
    layout->addRow(tr("Retraction speed"), new QLineEdit(this));
    layout->addRow(tr("Retraction distance"), new QLineEdit(this));

	extrusionMultBox = new QDoubleSpinBox();
	extrusionMultBox->setRange(0.25f, 2.5f);
	extrusionMultBox->setValue(1.0f);
	extrusionMultBox->setSingleStep(0.05);
	layout->addRow(tr("Extrusion multiplier"), extrusionMultBox);

	infillBox = new QSpinBox();
	infillBox->setValue(15);
	layout->addRow(tr("Infill spacing"), infillBox);

	wallsBox = new QSpinBox();
	wallsBox->setValue(2);
	layout->addRow(tr("Walls"), wallsBox);

	floorroofBox = new QSpinBox();
	floorroofBox->setValue(2);
	layout->addRow(tr("Floors / Roofs"), floorroofBox);

	return w;
}

QWidget *SlicerOptions::objectSettings() {
    auto w = new QWidget(this);
    auto layout = new QFormLayout(w);

    layout->addRow(tr("X offset"), new QLineEdit(this));
    layout->addRow(tr("Y offset"), new QLineEdit(this));
	// Auto calculate
    // layout->addRow(tr("Z offset"), new QLineEdit(this));
    layout->addRow(tr("X scale"), new QLineEdit(this));
    layout->addRow(tr("Y scale"), new QLineEdit(this));
    layout->addRow(tr("Z scale"), new QLineEdit(this));

	return w;
}

QWidget *SlicerOptions::printerSettings() {
    auto w = new QWidget(this);
    auto layout = new QFormLayout(w);

	nozzleTempBox = new QSpinBox();
	nozzleTempBox->setRange(185, 300);
	nozzleTempBox->setValue(200);
    layout->addRow(tr("Nozzle temperature"), nozzleTempBox);
	bedTempBox = new QSpinBox();
	bedTempBox->setValue(50);
    layout->addRow(tr("Bed temperature"), bedTempBox);
	bedXBox = new QSpinBox();
	bedXBox->setRange(100, 1000);
	bedXBox->setValue(235);
    layout->addRow(tr("Bed X (mm)"), bedXBox);
	bedYBox = new QSpinBox();
	bedYBox->setRange(100, 1000);
	bedYBox->setValue(235);
    layout->addRow(tr("Bed Y (mm)"), bedYBox);
	wallSpeedBox = new QSpinBox();
	wallSpeedBox->setRange(10, 150);
	wallSpeedBox->setValue(25);
    layout->addRow(tr("Wall print speed (mm/s)"), wallSpeedBox);
	infillSpeedBox = new QSpinBox();
	infillSpeedBox->setRange(10, 150);
	infillSpeedBox->setValue(50);
    layout->addRow(tr("Infill print speed (mm/s)"), infillSpeedBox);
    layout->addRow(tr("Z height"), new QLineEdit(this));
    layout->addRow(tr("Nozzle width"), new QLineEdit(this));
    layout->addRow(tr("Show printer bounds"), new QCheckBox(this));

	return w;
}
