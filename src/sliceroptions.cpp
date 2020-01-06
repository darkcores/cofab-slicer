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

	supportBox = new QCheckBox();
	layout->addRow("Support structures", supportBox);
	retractSpeedBox = new QSpinBox();
	retractSpeedBox->setValue(50);
    layout->addRow(tr("Retraction speed (mm/s)"), retractSpeedBox);
	retractDistanceBox = new QDoubleSpinBox();
	retractDistanceBox->setRange(0.0f, 20.0f);
	retractDistanceBox->setValue(4.0f);
    layout->addRow(tr("Retraction distance (mm)"), retractDistanceBox);
	retractRestoreBox = new QDoubleSpinBox();
	retractRestoreBox->setRange(0.0f, 20.0f);
	retractRestoreBox->setValue(3.99f);
    layout->addRow(tr("Retraction restore (mm)"), retractRestoreBox);

	coastingBox = new QDoubleSpinBox();
	coastingBox->setRange(0.0f, 10.0f);
	coastingBox->setValue(0.0f);
	coastingBox->setSingleStep(0.01f);
	layout->addRow(tr("Coasting distance (mm)"), coastingBox);

	nozzleWidthBox = new QDoubleSpinBox();
	nozzleWidthBox->setRange(0.1, 1.0);
	nozzleWidthBox->setValue(0.4);
	layout->addRow(tr("Nozzle width (mm)"), nozzleWidthBox);

	layerHeightBox = new QDoubleSpinBox();
	layerHeightBox->setRange(0.04, 1.0);
	layerHeightBox->setValue(0.2);
	layout->addRow(tr("Layer height (mm)"), layerHeightBox);

	extrusionMultBox = new QDoubleSpinBox();
	extrusionMultBox->setRange(0.25f, 2.5f);
	extrusionMultBox->setValue(1.05f);
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
	nozzleTempBox->setValue(205);
    layout->addRow(tr("Nozzle temperature"), nozzleTempBox);
	bedTempBox = new QSpinBox();
	bedTempBox->setValue(55);
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
	infillSpeedBox->setValue(40);
    layout->addRow(tr("Infill print speed (mm/s)"), infillSpeedBox);

	return w;
}
