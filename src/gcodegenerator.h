#ifndef GCODEGENERATOR_H
#define GCODEGENERATOR_H

#include "model3d.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

#include <QLine>
#include <QPoint>
#include <QPointF>
#include <QPolygon>
#include <QRect>
#include <QVector3D>

#include <fstream>
#include <iostream>
using namespace std;
const long INT_SCALE = 1000;

class GCodeGenerator {
  public:
    GCodeGenerator();
    void generateGcode(const std::vector<std::vector<QPolygon>> slices,
                       const string filename);
    void setLayerHeight(const double height);

  private:
    string getStartSequence(const int bedTemperature,
                            const int nozzleTemperature);
    string getEndSequence();
    string getGcodeSlice(std::vector<QPolygon> slice, double z);
    string getVectorMovementXY(double X, double Y, double extrusion);
    QPoint getFirstCollisionX(QPolygon polygon, QPoint start);
    QPoint getLastCollisionX(QPolygon polygon, QPoint start);

    std::vector<std::vector<QPolygon>> m_slices;
    double m_extrusion;
    double m_zstep;
};

#endif // GCODEGENERATOR_H
