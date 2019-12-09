#include "gcodegenerator.h"

GCodeGenerator::GCodeGenerator()
{
}

void GCodeGenerator::setLayerHeight(double height){
  if(height > 0){
    m_zstep = height;
  }
}

void GCodeGenerator::generateGcode(const std::vector<std::vector<QPolygon>> slices, const string filename){
  ofstream file (filename);
  //add start gcode (heating + cleaning)
  file << getStartSequence(50,200);
  //print every layer
  double z = 0;
  m_extrusion = 0;
  for(auto i: slices){
   file << getGcodeSlice(i, z);
   z= z + 0.2;
 }
  //add end gcode (cooldown + move bed)
  file << getEndSequence();
  file.close();
}

//get the movement needed for one slice
string GCodeGenerator::getGcodeSlice(const std::vector<QPolygon> slice, double z){
  string out ="";
  QPoint lastVisitedPoint = QPoint(0,0);
  m_zstep = 0.2;
  double distance = 0;
  double filament_diameter = 1.75;
  //(layer height * Nozzle diameter * L)/ 1.75
  double minimumVolume = zstep * 3.14 * (0.5 * 0.5)/4;
  double area = 3.14 * pow(m_zstep/2, 2) + m_zstep* (0.36 - m_zstep);
  //0.0634
  //for each polygon generate movement
  lastVisitedPoint = slice.front().front();
  for(auto i: slice){
    distance = area * sqrt(pow(lastVisitedPoint.x()/INT_SCALE - i.x()/INT_SCALE, 2) + pow(lastVisitedPoint.y()/INT_SCALE - i.y()/INT_SCALE ,2)) / filament_diameter;
    //move to starting point of the polygon
    QPoint startPolygon = i.front();
    out += getVectorMovementXY(startPolygon.x()/INT_SCALE, startPolygon.y()/INT_SCALE, m_extrusion);
    //connect each vertex of the polygon
    for(auto j: i){
      //layer thickness = 0.2 ==> 0.36 mm wide
      //(layer height * Nozzle diameter * L)/ 1.75
      m_extrusion += (m_zstep * 0.4 * sqrt(pow(lastPoint.x()/INT_SCALE - j.x()/INT_SCALE, 2) + pow(lastPoint.y()/INT_SCALE - j.y()/INT_SCALE ,2)))/1.75;
      out += getVectorMovementXY(j.x()/INT_SCALE, j.y()/INT_SCALE, m_extrusion);      //cout << j.x() << ", " << j.y();
    }
    // add line back to starting point
    out += getVectorMovementXY(startPolygon.x()/INT_SCALE, startPolygon.y()/INT_SCALE, m_extrusion);
    distance = 0;
  }
  return out;
}


string GCodeGenerator::getVectorMovementXY(double X, double Y, double extrusion){
  return "G1 X" + std::to_string(X) + " Y" + std::to_string(Y) + " E" + std::to_string(extrusion) + " ; move \n";
}

QPoint GCodeGenerator::getFirstCollisionX(QPolygon polygon, QPoint start){
  QRect r= polygon.boundingRect();
  for(double i = r.left()/INT_SCALE +1; i<r.right()/INT_SCALE + 1; i++){
    if(polygon.containsPoint(QPoint(i,start.y()), Qt::OddEvenFill)){
      //cout << i <<", " << start.y()<<"\n";
      return QPoint(i, start.y());
    }
  }
  return QPoint(0,0);
}

QPoint GCodeGenerator::getLastCollisionX(QPolygon polygon, QPoint start){
  QRect r= polygon.boundingRect();
  //cout << start.x() <<", " << start.y()<<"\n";
  for(double i = start.x(); i<r.right()/INT_SCALE + 1; i++){
    //OddEvenFill error?
    if(!polygon.containsPoint(QPoint(i,start.y()), Qt::OddEvenFill)){
      cout << "end = " << i <<", " << start.y()<<"\n";
      return QPoint(i-1, start.y());
    }
  }
  return start;
}

//heat the bed/nozzle + extrude a bit
string GCodeGenerator::getStartSequence(const int bedTemperature,const int nozzleTemperature){
  string out = "";

  out += "M104 S" + std::to_string(nozzleTemperature) + " ; set temp nozzle but do not wait\nM190 S" + std::to_string(bedTemperature) + " ; set bed temp and wait\n" + "M109 S" + std::to_string(nozzleTemperature) +"; wait until temps are reached\n";
  out += "M201 X500.00 Y500.00 Z100.00 E5000.00 ;Setup machine max acceleration\n";
  out += "M203 X500.00 Y500.00 Z10.00 E50.00 ;Setup machine max feedrate\n";
  out += "M204 P500.00 R1000.00 T500.00 ;Setup Print/Retract/Travel acceleration\n";
  out += "M205 X8.00 Y8.00 Z0.40 E5.00 ;Setup Jerk\n";
  out += "M220 S100 ;Reset Feedrate\n";
  out += "M221 S100 ;Reset Flowrate\n";
  out += "G28 ;Home\n";
  out += "G92 E0 ;Reset Extruder\n";
  out += "G1 Z2.0 F3000 ;Move Z Axis up\n";
  out += "G1 X10.1 Y20 Z0.28 F5000.0 ;Move to start position\n";
  out += "G1 X10.1 Y200.0 Z0.28 F1500.0 E15 ;Draw the first line\n";
  out += "G1 X10.4 Y200.0 Z0.28 F5000.0 ;Move to side a little\n";
  out += "G1 X10.4 Y20 Z0.28 F1500.0 E30 ;Draw the second line\n";
  out += "G92 E0 ;Reset Extruder\n";
  out += "G1 Z2.0 F3000 ;Move Z Axis up\n";
  out += "G21 ;metricvalues\nG90 ; absolute positioning\nM82 ; extruder to absolute mode\n";

  //out += "G28 X0 Y0 ; X,Y to min endstops \nG28 Z0 ; z to min endstops \nG1 Z15.0 F9000 ; move platform down 15 mm \nG92 E0 ; zero extruded length \nG1 F200 E3 ; extrude 3mm \nG92 E0 ; zero extrude length \nG1 F9000 ; add text printing on lcd \nM 117 Printing\nG1 F1000\n";

  //end sequence out +=   "G91 ;Relative positioning\nG1 E-2 F2700 ;Retract a bit\nG1 E-2 Z0.2 F2400 ;Retract and raise Z \nG1 X5 Y5 F3000 ;Wipe out \nG1 Z10 ;Raise Z more\nG90 ;Absolute positionning";

  return out;
}

//stop heating + move bed
string GCodeGenerator::getEndSequence(){
  return "G91 ; set relative pos mode \nG1 E-5 F600 retract filament \nM140 S0 ; turn off bed heating \nM104 S0 ; turn off nozzle heating \nG28 X ; home x-axis \nG0 Y280 F600; bring bed to the front \nM84; stop stepper motors";
}
