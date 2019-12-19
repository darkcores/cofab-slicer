#ifndef SLICEROPTIONS_H
#define SLICEROPTIONS_H

#include <QDockWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>

class QCheckBox;
class QLineEdit;

class SlicerOptions : public QDockWidget {
    Q_OBJECT
  public:
    explicit SlicerOptions(QWidget *parent = nullptr);

	int infill() { return infillBox->value(); }
	int walls() { return wallsBox->value(); }
	int floorroof() { return floorroofBox->value(); }

  private:
    QWidget *objectSettings();
    QWidget *slicerSettings();
    QWidget *printerSettings();

	QSpinBox *infillBox, *wallsBox, *floorroofBox;

  signals:

  public slots:
};

#endif // SLICEROPTIONS_H
