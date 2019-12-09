#include <QPoint>
#include <QPolygon>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const double area = M_PI * pow(0.2 / 2, 2) + 0.2 * (0.36 - 0.2);
const double extrusion_scale = 1.0;
// const double retraction = (5 * area * extrusion_scale);
const double retraction = 4;

void startcode(std::ofstream &o) {
    o << "M190 S50\n";
    o << "M109 S200\n";
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

void gotoPoint(std::ofstream &o, QPoint &p, double z) {
    o << "G0 F5000 X" << std::to_string(p.x() / 1000.0f) << " Y"
      << std::to_string(p.y() / 1000.0f) << " Z" << std::to_string(z) << "\n";
}

double distance(const QPoint &from, const QPoint &to) {
    const double diffX = (from.x() / 1000.0f) - (to.x() / 1000.0f);
    const double diffY = (from.y() / 1000.0f) - (to.y() / 1000.0f);
    const double length = sqrt(pow(diffX, 2) + pow(diffY, 2));
    return length;
}

void printLine(std::ofstream &o, QPoint &from, QPoint &to, double &extrusion,
               bool coast = false) {
    // extrusion += (4 * 0.2 * 0.95 * length) / (M_PI * 0.4);
    double length = distance(from, to);
    if (coast) {
        if (length <= 0.5) { // Don't extrude on last part
			// Test: just do nothing
			/*
            o << "G1 X" << std::to_string(to.x() / 100.0f) << " Y"
              << std::to_string(to.y() / 100.0f) << "\n";
			*/
        } else { // extrude only beginning of line
			// Test: just go to almost the beginning
            extrusion += ((length - 0.6) * area * extrusion_scale) / 1.75;
            const double coastscale = length / (length - 0.4);
			double coastX;
			double coastY;
			coastX = ((to.x() - from.x()) / 1000.0f) / coastscale;
			coastY = ((to.y() - from.y()) / 1000.0f) / coastscale;
			// o << "; Coasting\n";
            o << "G1 X" << std::to_string((from.x() / 1000.0f) + coastX)
              << " Y" << std::to_string((from.y() / 1000.0f) + coastY) << " E"
              << std::to_string(extrusion) << "\n";
			/*
            o << "G1 X" << std::to_string(to.x() / 100.0f) << " Y"
              << std::to_string(to.y() / 100.0f) << "\n";
			*/
        }
    } else {
		if (length <= 0.4)
			length /= 2;
		else
			length -= 0.2;
        extrusion += (length * area * extrusion_scale) / 1.75;
        o << "G1 X" << std::to_string(to.x() / 1000.0f) << " Y"
          << std::to_string(to.y() / 1000.0f) << " E"
          << std::to_string(extrusion) << "\n";
    }
}

void printPath(std::ofstream &o, QPolygon &poly, double &extrusion, double z) {
    QPoint p = poly[0];
    if (z < 0.4) {
        o << "G1 F1300\n";
    } else {
        if (poly.size() > 2)
            o << "G1 F1800\n";
        else
            o << "G1 F3400\n"; // Infill can be faster
    }
    for (auto &pt : poly) {
        printLine(o, p, pt, extrusion);
        p = pt;
    }
    if (poly.size() > 2) {
        printLine(o, p, poly[0], extrusion, true);
    }

    // o << "G1 F2700 E" << std::to_string(extrusion - retraction) << "\n";
}

void exportSlices(std::vector<std::vector<QPolygon>> &slices) {
    std::cout << "Retraction: " << std::to_string(retraction) << std::endl;
    std::ofstream o("test.gcode");
    double extrusion = 0;
    startcode(o);
    double z = 0.2;
    for (auto &slice : slices) {
        o << "; Layer: " << std::to_string(z) << "\n";
        if (z > 0.3 && z < 0.5)
            o << "M106 S85\n";
        if (z > 0.5 && z < 0.7)
            o << "M106 S170\n";
        if (z > 0.7 && z < 0.9)
            o << "M106 S256\n";
        // Go to start quickly
        bool retracted = true;
        for (std::size_t i = 0; i < slice.size(); i++) {
            QPoint p = slice[i][0];
            QPoint nextp(0, 0);
            if (i < (slice.size() - 1)) {
                nextp = slice[i + 1][0];
            }
            gotoPoint(o, p, z);
            if (retracted) {
                // o << "G1 F2700 E" << std::to_string(extrusion) << "\n";
                o << "G1 F2700 E" << extrusion << "\n";
                retracted = false;
            }
            printPath(o, slice[i], extrusion, z);
            // o << "; Distance to next: "
            //   << std::to_string(distance(nextp, slice[i].last())) << "\n";
            if (distance(nextp, slice[i].last()) > 0.7) {
                // No retraction if very close
                o << "G1 F2700 E" << std::to_string(extrusion - retraction)
                  << "\n";
                retracted = true;
            }
        }
        z += 0.2;
    }
    endcode(o);
    o.close();
}
