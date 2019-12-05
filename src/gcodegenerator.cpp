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
  m_zstep = 0.2;
  //(layer height * Nozzle diameter * L)/ 1.75

  //double minimumVolume = zstep * 3.1415 * (0.5 * 0.5)/4;
  double area = 3.1415 * pow(m_zstep/2, 2) + m_zstep* (0.36 - m_zstep);

  //lift up
  out += "G0 Z" + std::to_string(2) + " ; lift up between polygons\n";

  //for each polygon generate movement
  for(auto i: slice){
    //nozzle down
    out += "G0 Z" + std::to_string(-2) + " ; lift up between polygons\n";

    //move to starting point of the polygon
    QPoint startPolygon = i.front();
    out += getVectorMovementXY(startPolygon.x()/INT_SCALE, startPolygon.y()/INT_SCALE, m_extrusion);
    QPoint lastPoint = i.front();

    //connect each vertex of the polygon
    for(auto j: i){
      //layer thickness = 0.2 ==> 0.36 mm wide
      m_extrusion +=  sqrt(pow(lastPoint.x()/INT_SCALE - j.x()/INT_SCALE, 2) + pow(lastPoint.y()/INT_SCALE - j.y()/INT_SCALE ,2));
      out += getVectorMovementXY(j.x()/INT_SCALE, j.y()/INT_SCALE, m_extrusion);      //cout << j.x() << ", " << j.y();
      lastPoint = j;
    }
    // add line back to starting point
    out += getVectorMovementXY(startPolygon.x()/INT_SCALE, startPolygon.y()/INT_SCALE, m_extrusion);
    //fill the polygon
    //out+= getFullyFilledPolygon(i);

    //lift up
    out += "G0 Z" + std::to_string(2) + " ; lift up between polygons\n";
  }
  //move up
  out += "G0 Z" + std::to_string(z) + " ; move to next layer\n";
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
string GCodeGenerator::getStartSequence(int bedTemperature, int nozzleTemperature){
  string out = "G21 ;metricvalues\nG90 ; absolute positioning\nM82 ; extruder to absolute mode \nM107 ; start with fan off \n";

  /*
  G1 X0 Y{machine_depth} ;Present print
  M106 S0 ;Turn-off fan
  M104 S0 ;Turn-off hotend
  M140 S0 ;Turn-off bed
  */

  out += "M104 S" + std::to_string(nozzleTemperature) + " ; set temp nozzle but do not wait\nM190 S" + std::to_string(bedTemperature) + " ; set bed temp and wait\n" + "M109 S" + std::to_string(nozzleTemperature) +"; wait until temps are reached\n";

  //out += "G28 X0 Y0 ; X,Y to min endstops \nG28 Z0 ; z to min endstops \nG1 Z15.0 F9000 ; move platform down 15 mm \nG92 E0 ; zero extruded length \nG1 F200 E3 ; extrude 3mm \nG92 E0 ; zero extrude length \nG1 F9000 ; add text printing on lcd \nM 117 Printing\nG1 F1000\n";
  out +=   "G91 ;Relative positionning\nG1 E-2 F2700 ;Retract a bit\nG1 E-2 Z0.2 F2400 ;Retract and raise Z \nG1 X5 Y5 F3000 ;Wipe out \nG1 Z10 ;Raise Z more\nG90 ;Absolute positionning";

  return out;
}

//stop heating + move bed
string GCodeGenerator::getEndSequence(){
  return "G91 ; set relative pos mode \nG1 E-5 F600 retract filament \nM140 S0 ; turn off bed heating \nM104 S0 ; turn off nozzle heating \nG28 X ; home x-axis \nG0 Y280 F600; bring bed to the front \nM84; stop stepper motors";
}
