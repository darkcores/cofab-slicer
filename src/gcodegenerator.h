#ifndef GCODEGENERATOR_H
#define GCODEGENERATOR_H

#include "model3d.h"
#include <algorithm>
#include <array>
#include <string>
#include <vector>
#include <cmath>

#include <QLine>
#include <QPoint>
#include <QPointF>
#include <QPolygon>
#include <QVector3D>
#include <QRect>

#include <iostream>
#include <fstream>
using namespace std;
const long INT_SCALE = 1000000;

class GCodeGenerator{
public:
  GCodeGenerator();
  string generateGcode(std::vector<std::vector<QPolygon>> slices);

private:
  string getStartSequence(int bedTemperature, int nozzleTemperature);
  string getEndSequence();
  string getGcodeSlice(std::vector<QPolygon> slice, double z);
  string getVectorMovementXY(double X, double Y, double extrusion);
  string getFullyFilledPolygon(QPolygon polygon);
  QPoint getFirstCollisionX(QPolygon polygon, QPoint start);
  QPoint getLastCollisionX(QPolygon polygon, QPoint start);

  std::vector<std::vector<QPolygon>> m_slices;
  double m_extrusion;
  double m_zstep;
};

#endif // GCODEGENERATOR_H
