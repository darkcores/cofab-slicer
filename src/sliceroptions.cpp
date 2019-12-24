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

    layout->addRow(tr("Print temperature"), new QLineEdit(this));
    layout->addRow(tr("Print speed"), new QLineEdit(this));
    layout->addRow(tr("Retraction"), new QCheckBox(this));
    layout->addRow(tr("Retraction speed"), new QLineEdit(this));
    layout->addRow(tr("Retraction distance"), new QLineEdit(this));

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

    layout->addRow(tr("Bed X"), new QLineEdit(this));
    layout->addRow(tr("Bed Y"), new QLineEdit(this));
    layout->addRow(tr("Z height"), new QLineEdit(this));
    layout->addRow(tr("Nozzle width"), new QLineEdit(this));
    layout->addRow(tr("Show printer bounds"), new QCheckBox(this));

	return w;
}
