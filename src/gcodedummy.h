#include <QPoint>
#include <QPolygon>
#include <QVector2D>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// const double area = M_PI * pow(0.2 / 2, 2) + 0.2 * (0.36 - 0.2) / (M_PI / 4 *
// pow(1.75, 2));
const double extrusion_scale = 1.05;
// const double A = ((0.4 - 0.2) * 0.2) + (M_PI * pow(0.2 / 2, 2));
// const double area = (A * 4) / (M_PI * pow(1.75, 2));
// Fun stuff: https://manual.slic3r.org/advanced/flow-math
// Modified from there
const double areaLine = ((0.2 * (0.42 - 0.2)) + (M_PI * pow(0.2 / 2, 2))) /
                        (M_PI * pow(1.75 / 2, 2));
// const double areaInfill = (M_PI * pow(0.4 / 2, 2)) / (M_PI * pow(1.75 / 2, 2));
const double areaInfill = areaLine * 1.5;
double area = areaLine;
// const double retraction = (5 * area * extrusion_scale);
const double retraction = 4;
const double recover = 3.999;

void startcode(std::ofstream &o) {
    o << "M190 S55\n";
    o << "M109 S203\n";
    o << "M106 S0\n";
    o << "G21 ;metricvalues\n";
    o << "M201 X500.00 Y500.00 Z100.00 E5000.00 ;Setup machine max "
         "acceleration\n";
    o << "M203 X500.00 Y500.00 Z10.00 E50.00 ;Setup machine max feedrate\n";
    o << "M204 P500.00 R1000.00 T500.00 ;Setup Print/Retract/Travel "
         "acceleration\n";
    o << "M207 F5000 S5";
    o << "M205 X8.00 Y8.00 Z0.40 E5.00 ;Setup Jerk\n";
    o << "M220 S100 ;Reset Feedrate\n";
    o << "M221 S100 ;Reset Flowrate\n";
    o << "G28 ;Home\n";
    o << "G92 E0 ;Reset Extruder\n";
    o << "G1 Z2.0 F3000 ;Move Z Axis up\n";
    o << "G1 X10.1 Y20 Z0.28 F5000.0 ;Move to start position\n";
    o << "G1 X10.1 Y200.0 Z0.28 F1500.0 E15 ;Draw the first line\n";
    o << "G1 X10.4 Y200.0 Z0.28 F5000.0 ;Move to side a little\n";
    o << "G1 X10.4 Y20 Z0.28 F1500.0 E30 ;Draw the second line\n";
    o << "G92 E0 ;Reset Extruder\n";
    o << "G90 ; absolute positioning\n";
    o << "M82 ; extruder to absolute mode\n";
    o << "G1 F2700 E-" << std::to_string(retraction) << "\n";
    // o << "G1 Z2.0 F3000 ;Move Z Axis up\n";
    // o << "G1 F2700 E-" << std::to_string(retraction) << "\n";
}

void endcode(std::ofstream &o) {
    // o << "G91 ; set relative pos mode \n"
    // << "G1 E-5 F600 retract filament \n"
    o << "M140 S0 ; turn off bed heating \n"
      << "M104 S0 ; turn off nozzle heating \n"
      << "G0 X5 F600 ; home x-axis \n"
      << "G0 Y280 F600; bring bed to the front \n "
      << "M106 S0; stop fan\n"
      << "M84; stop stepper motors";
}

void gotoPoint(std::ofstream &o, QPoint &p, double z = -1) {
    if (z > 0) {
        o << "G0 F5000 X" << std::to_string(p.x() / 1000.0f) << " Y"
          << std::to_string(p.y() / 1000.0f) << " Z" << std::to_string(z)
          << "\n";
    } else {
        o << "G0 F5000 X" << std::to_string(p.x() / 1000.0f) << " Y"
          << std::to_string(p.y() / 1000.0f) << "\n";
    }
}

double distance(const QPoint &from, const QPoint &to) {
    // const double diffX = (from.x() / 1000.0f) - (to.x() / 1000.0f);
    // const double diffY = (from.y() / 1000.0f) - (to.y() / 1000.0f);
    // const double length = sqrt(pow(diffX, 2) + pow(diffY, 2));
    const double length = QVector2D(to - from).length() / 1000;
    return length;
}

double polyLength(const QPolygon &poly) {
    QPoint previous = poly[0];
    double len = 0;
    for (int i = 1; i < poly.size(); i++) {
        len += distance(previous, poly[i]);
        previous = poly[i];
    }
    return len;
}

double printLine(std::ofstream &o, QPoint &from, QPoint &to, double &extrusion,
                 bool coast = false, double offset = 0) {
    // extrusion += (4 * 0.2 * 0.95 * length) / (M_PI * 0.4);
    double length = distance(from, to);
    if (coast) {
        if (length <= 0.4) { // Don't extrude on last part
                             // Test: just do nothing
            o << "X" << std::to_string(to.x() / 1000.0f) << " Y"
              << std::to_string(to.y() / 1000.0f) << "\n";
        } else { // extrude only beginning of line
                 // Test: just go to almost the beginning
            extrusion += ((length - 0.8) * area * extrusion_scale);
            const double coastscale = length / (length - 1.4);
            double coastX;
            double coastY;
            coastX = ((to.x() - from.x()) / 1000.0f) / coastscale;
            coastY = ((to.y() - from.y()) / 1000.0f) / coastscale;
            // o << "; Coasting\n";
            o << "X" << std::to_string((from.x() / 1000.0f) + coastX) << " Y"
              << std::to_string((from.y() / 1000.0f) + coastY) << " E"
              << std::to_string(extrusion) << "\n";
            o << "G1 X" << std::to_string(to.x() / 1000.0f) << " Y"
              << std::to_string(to.y() / 1000.0f) << "\n";
            /*
o << "G1 X" << std::to_string(to.x() / 100.0f) << " Y"
  << std::to_string(to.y() / 100.0f) << "\n";
            */
        }
    } else {
        if (offset > 0) {
            length -= std::min((3 * (1 - offset)), (length * (1 - offset)));
            /*
if (length <= 0.2)
    length *= 0.35;
else if (length <= 0.4)
    length *= 0.65;
else if (length <= 0.6)
    length *= 0.90;
else
    length -= 0.2;
            */
        }
        extrusion += (length * area * extrusion_scale);
        o << "X" << std::to_string(to.x() / 1000.0f) << " Y"
          << std::to_string(to.y() / 1000.0f) << " E"
          << std::to_string(extrusion) << "\n";
    }
    return length;
}

float getAngle(const QPoint p, const QPoint last, const QPoint next) {
    if (last == QPoint(0, 0) || next == QPoint(0, 0)) {
        return 0;
    }
    QVector2D v1(p - last), v2(p - next);
    float dot = QVector2D::dotProduct(v1, v2);
    float angle = dot / (v1.length() * v2.length());
    return angle;
}

QPoint printPath(std::ofstream &o, QPolygon &poly, double &extrusion,
                 double z) {
    QPoint p = poly[0], prevp(0, 0);
    double length = polyLength(poly);
    if (z < 0.4) {
        o << "G1 F1300 ";
    } else {
        if (poly.size() > 2)
            o << "G1 F1800 ";
        else
            o << "G1 F2400 "; // Infill can be faster
                              // TODO split infill & floors/roofs
    }
    bool offset = poly.size() > 2;
    if (offset) {
        area = areaLine;
    } else {
        area = areaInfill;
    }
    // for (auto &pt : poly) {
    for (int i = 1; i < poly.size(); i++) {
        auto pt = poly[i];
        auto angle = getAngle(p, prevp, pt);
        if (length <= 0.4 && offset) {
            double l = printLine(o, p, pt, extrusion, true);
            length -= l;
        } else {
            double l = printLine(o, p, pt, extrusion, false, angle);
            length -= l;
        }
        prevp = pt;
        p = pt;
        if (i < (poly.size() - 1)) {
            o << "G1 ";
        }
    }
    if (poly.size() > 2) {
        o << "G1 ";
        printLine(o, p, poly[0], extrusion, true);
        return poly[0];
    } else {
        return p;
    }

    // o << "G1 F2700 E" << std::to_string(extrusion - retraction) << "\n";
}

void exportSlices(std::vector<std::vector<QPolygon>> &slices) {
    /*
    float testangle = getAngle(QPoint(1, 1), QPoint(0, 1), QPoint(-1, 2));
    std::cout << "Angle test value: " << testangle << std::endl;
    */
    std::cout << "Area line: " << areaLine << " Area infill: " << areaInfill
              << std::endl;
    std::cout << "Retraction: " << std::to_string(retraction) << std::endl;
    std::ofstream o("test.gcode");
    double extrusion = -retraction;
    startcode(o);
    double z = 0.2;
    // for (auto &slice : slices) {
    bool retracted = true;
    for (std::size_t s = 0; s < slices.size(); s++) {
        auto slice = slices[s];
        o << "; Layer: " << std::to_string(z) << "\n";
        if (z > 0.3 && z < 0.5)
            o << "M106 S85\n";
        if (z > 0.5 && z < 0.7)
            o << "M106 S170\n";
        if (z > 0.7 && z < 0.9)
            o << "M106 S256\n";
        // Go to start quickly
        for (std::size_t i = 0; i < slice.size(); i++) {
            QPoint p = slice[i][0];
            QPoint nextp(0, 0);
            if (i < (slice.size() - 1)) {
                nextp = slice[i + 1][0];
            } else if (s < (slices.size() - 1)) {
                nextp = slices[s][0][0];
            }
            if (i == 0)
                gotoPoint(o, p, z);
            else
                gotoPoint(o, p);
            if (retracted) {
                // o << "G1 F2700 E" << std::to_string(extrusion) << "\n";
                extrusion += recover;
                o << "G1 F2700 E" << extrusion << "\n";
                retracted = false;
            }
            QPoint end = printPath(o, slice[i], extrusion, z);
            // o << "; Distance to next: "
            //   << std::to_string(distance(nextp, slice[i].last())) << "\n";
            if (distance(nextp, end) > 1.3) {
                // No retraction if very close
                extrusion -= retraction;
                o << "G1 F2700 E" << std::to_string(extrusion) << "\n";
                retracted = true;
            }
        }
        z += 0.2;
    }
    endcode(o);
    o.close();
}
