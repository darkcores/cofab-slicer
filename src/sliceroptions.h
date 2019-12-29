#ifndef SLICEROPTIONS_H
#define SLICEROPTIONS_H

#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>

class QCheckBox;
class QLineEdit;

class SlicerOptions : public QDockWidget {
    Q_OBJECT
  public:
    explicit SlicerOptions(QWidget *parent = nullptr);

    int infill() { return infillBox->value(); }
    int walls() { return wallsBox->value(); }
    int floorroof() { return floorroofBox->value(); }
    int nozzleTemp() { return nozzleTempBox->value(); }
    int bedTemp() { return bedTempBox->value(); }
	int bedX() { return bedXBox->value(); }
	int bedY() { return bedYBox->value(); }
	int wallSpeed() { return wallSpeedBox->value() * 60; }
	int infillSpeed() { return infillSpeedBox->value() * 60; }
	double coasting() { return coastingBox->value(); }
	double extrusionMult() { return extrusionMultBox->value(); }
	int retractSpeed() { return retractSpeedBox->value() * 60; }
	double retractDistance() { return retractDistanceBox->value(); }
	double retractRestore() { return retractRestoreBox->value(); }
	double layerHeight() { return layerHeightBox->value(); }
	double nozzleWidth() { return nozzleWidthBox->value(); }

  private:
    QWidget *objectSettings();
    QWidget *slicerSettings();
    QWidget *printerSettings();

    QSpinBox *infillBox, *wallsBox, *floorroofBox;
    QDoubleSpinBox *extrusionMultBox, *coastingBox;
    QSpinBox *nozzleTempBox, *bedTempBox, *bedXBox, *bedYBox;
	QSpinBox *wallSpeedBox, *infillSpeedBox, *retractSpeedBox;
	QDoubleSpinBox *retractDistanceBox,  *retractRestoreBox;
	QDoubleSpinBox *nozzleWidthBox, *layerHeightBox;

  signals:

  public slots:
};

#endif // SLICEROPTIONS_H
