#include "sliceroptions.h"

#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QFormLayout>

SlicerOptions::SlicerOptions(QWidget *parent) : QDockWidget(parent) {
	auto w = new QWidget(this);
	auto layout = new QFormLayout(w);

	layout->addRow(tr("Print temperature"), new QLineEdit(this));
	layout->addRow(tr("Print speed"), new QLineEdit(this));
	layout->addRow(tr("Retraction"), new QCheckBox(this));
	layout->addRow(tr("Retraction speed"), new QLineEdit(this));
	layout->addRow(tr("Retraction distance"), new QLineEdit(this));

	setWidget(w);
}
