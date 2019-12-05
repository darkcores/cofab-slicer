#include "gcodegenerator.h"

GCodeGenerator::GCodeGenerator()
{

}


string GCodeGenerator::generateGcode(std::vector<std::vector<QPolygon>> slices){
  //ofstream file ("test.gcode");
  //add start gcode (heating + cleaning)
  string out = "";
  out += getStartSequence(50,200);
  //print every layer
  double z = 0;
  m_extrusion = 0;
  for(auto i: slices){
   out += getGcodeSlice(i, z);
   z= z + 0.2;
 }
  //add end gcode (cooldown + move bed)
  out += getEndSequence();
}

//get the movement needed for one slice
string GCodeGenerator::getGcodeSlice(std::vector<QPolygon> slice, double z){
  string out ="";
  m_zstep = 0.2;
  //double minimumVolume = zstep * 3.1415 * (0.5 * 0.5)/4;
  double area = 3.1415 * pow(m_zstep/2, 2) + m_zstep* (0.36 - m_zstep);

  //for each polygon generate movement
  for(auto i: slice){
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
  }
  //move up
  out += "G0 Z" + std::to_string(z) + " ; move to next layer\n";
  return out;
}


string GCodeGenerator::getVectorMovementXY(double X, double Y, double extrusion){
  return "G1 X" + std::to_string(X) + " Y" + std::to_string(Y) + " E" + std::to_string(extrusion) + " ; move \n";
}

string GCodeGenerator::getFullyFilledPolygon(QPolygon polygon){
  //scan line
  QRect r= polygon.boundingRect();
  string out ="";
  QPoint pStart = QPoint(0,0);
  QPoint pStop = QPoint(0,0);
  //cout << "y = " << (r.bottom())/INT_SCALE <<", from " << r.top()/INT_SCALE << "\n";
  //cout << "\n" <<getFirstCollisionX(polygon, QPoint(0,0)).x();
  //cout << getFirstCollisionX(polygon, QPoint(2,0)).x();
  //cout << "\n\n";

  for(double i = r.top()/INT_SCALE; i<r.bottom()/INT_SCALE - 1; i++){
    pStart = getFirstCollisionX(polygon, QPoint(r.left()/INT_SCALE, i));
    //cout << "firstx " <<pStart.x()/INT_SCALE << "\n";
    //move nozzle
    out+= getVectorMovementXY(pStart.x(), pStart.y(), m_extrusion);
    for(int j = pStart.x(); j<r.right()/INT_SCALE -1; j++){
      //find next border
      pStop = getLastCollisionX(polygon, QPoint(pStart.x(), i));
      //print
      m_extrusion +=  sqrt(pow(pStart.x() - pStop.x(), 2) + pow(pStart.y() - pStop.y(),2));
      //print to the end
      out+= getVectorMovementXY(pStop.x(), pStop.y(), m_extrusion);
      //update start
      pStart = getFirstCollisionX(polygon, QPoint(pStop.x(), i));
      //cout << "y = " << j <<", from " << pStart.x() << "to" << pStop.x()<< "\n";
    }
  }

  return out;
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
  out += "M104 S" + std::to_string(nozzleTemperature) + " ; set temp nozzle but do not wait\nM190 S" + std::to_string(bedTemperature) + " ; set bed temp and wait\n" + "M109 S" + std::to_string(nozzleTemperature) +"; wait until temps are reached\n";

  out += "G28 X0 Y0 ; X,Y to min endstops \nG28 Z0 ; z to min endstops \nG1 Z15.0 F9000 ; move platform down 15 mm \nG92 E0 ; zero extruded length \nG1 F200 E3 ; extrude 3mm \nG92 E0 ; zero extrude length \nG1 F9000 ; add text printing on lcd \nM 117 Printing\nG1 F1000\n";
  return out;
}

//stop heating + move bed
string GCodeGenerator::getEndSequence(){
  return "G91 ; set relative pos mode \nG1 E-5 F600 retract filament \nM140 S0 ; turn off bed heating \nM104 S0 ; turn off nozzle heating \nG28 X ; home x-axis \nG0 Y280 F600; bring bed to the front \nM84; stop stepper motors";
}
