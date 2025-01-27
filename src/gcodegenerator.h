#ifndef GCODEGENERATOR_H
#define GCODEGENERATOR_H

#include "model3d.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include <QLine>
#include <QPoint>
#include <QPolygon>
#include <QVector2D>

#include <fstream>
#include <iostream>
const long INT_SCALE = 1000;

class GCodeGenerator {
  public:
    GCodeGenerator(const std::string filename);
	~GCodeGenerator();
    void setLayerHeight(const double height);
	void setNozzleTemperature(int temp);
	void setBedTemperature(int temp);
	void setExtrusionMultiplier(double scale);
	void setWallSpeed(int speed);
	void setInfillSpeed(int speed);
	void setCoasting(double distance);
	void setRetractSpeed(int speed);
	void setRetractDistance(double distance);
	void setRetractRestore(double distance);
	void setNozzleWidth(double width);

	void exportSlices(std::vector<std::vector<QPolygon>> &slices);

  private:
	int retractSpeed = 2700;
	double nozzleWidth = 0.4;
	double coastingDistance = 0;
	int wallSpeed = 2000;
	int infillSpeed = 4000;
    double extrusion_scale = 1.20;
	// Based on: https://manual.slic3r.org/advanced/flow-math
    double areaLine = ((0.2 * (0.42 - 0.2)) + (M_PI * pow(0.2 / 2, 2))) /
                            (M_PI * pow(1.75 / 2, 2));
    double areaInfill = areaLine * 1.0;
	double area = areaLine;
	double retraction = 4;
    double recover = 3.999;
	double layerHeight = 0.2;
	int nozzleTemp = 200, bedTemp = 50;

    std::ofstream out;

    void startcode();
    void endcode();
    void gotoPoint(QPoint &p, double z = -1);
    double polyLength(const QPolygon &poly);
    double printLine(QPoint &from, QPoint &to, double &extrusion,
                     bool coast = false, double offset = 0);
    QPoint printPath(QPolygon &poly, double &extrusion, double z);

    inline double distance(const QPoint &from, const QPoint &to) {
        const double length = QVector2D(to - from).length() / 1000;
        return length;
    }

    inline float getAngle(const QPoint p, const QPoint last,
                          const QPoint next) {
        if (last == QPoint(0, 0) || next == QPoint(0, 0)) {
            return 0;
        }
        QVector2D v1(p - last), v2(p - next);
        float dot = QVector2D::dotProduct(v1, v2);
        float angle = dot / (v1.length() * v2.length());
        return angle;
    }
};

#endif // GCODEGENERATOR_H
