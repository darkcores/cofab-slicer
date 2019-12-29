#include "gcodegenerator.h"

GCodeGenerator::GCodeGenerator(const std::string filename) {
    out = std::ofstream(filename);
}

GCodeGenerator::~GCodeGenerator() {
    if (out.is_open())
        out.close();
}

void GCodeGenerator::setLayerHeight(const double height) {
    layerHeight = height;
    areaLine = ((height * ((nozzleWidth * 1.05) - height)) +
                (M_PI * pow(height / 2, 2))) /
               (M_PI * pow(1.75 / 2, 2));
    areaInfill = areaLine * 1.5;
}

void GCodeGenerator::setNozzleWidth(double width) {
    nozzleWidth = width;
    areaLine = ((layerHeight * ((nozzleWidth * 1.05) - layerHeight)) +
                (M_PI * pow(layerHeight / 2, 2))) /
               (M_PI * pow(1.75 / 2, 2));
    areaInfill = areaLine * 1.5;
}

void GCodeGenerator::setNozzleTemperature(int temp) { nozzleTemp = temp; }

void GCodeGenerator::setBedTemperature(int temp) { bedTemp = temp; }

void GCodeGenerator::setExtrusionMultiplier(double scale) {
    extrusion_scale = scale;
}

void GCodeGenerator::setCoasting(double distance) {
    coastingDistance = distance;
}

void GCodeGenerator::setWallSpeed(int speed) { wallSpeed = speed; }

void GCodeGenerator::setInfillSpeed(int speed) { infillSpeed = speed; }

void GCodeGenerator::setRetractSpeed(int speed) { retractSpeed = speed; }

void GCodeGenerator::setRetractDistance(double distance) {
    retraction = distance;
}

void GCodeGenerator::setRetractRestore(double distance) { recover = distance; }

void GCodeGenerator::exportSlices(std::vector<std::vector<QPolygon>> &slices) {
    /*
    float testangle = getAngle(QPoint(1, 1), QPoint(0, 1), QPoint(-1, 2));
    std::cout << "Angle test value: " << testangle << std::endl;
    */
    std::cout << "Area line: " << areaLine << " Area infill: " << areaInfill
              << std::endl;
    std::cout << "Retraction: " << std::to_string(retraction) << std::endl;
    // std::ofstream o("test.gcode");
    double extrusion = -retraction;
    startcode();
    double z = layerHeight;
    // for (auto &slice : slices) {
    bool retracted = true;
    for (std::size_t s = 0; s < slices.size(); s++) {
        auto slice = slices[s];
        out << "; Layer: " << std::to_string(z) << "\n";
        if (z > 0.3 && z < 0.5)
            out << "M106 S85\n";
        if (z > 0.5 && z < 0.7)
            out << "M106 S170\n";
        if (z > 0.7 && z < 0.9)
            out << "M106 S256\n";
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
                gotoPoint(p, z);
            else
                gotoPoint(p);
            if (retracted) {
                // o << "G1 F2700 E" << std::to_string(extrusion) << "\n";
                extrusion += recover;
                out << "G1 F" << retractSpeed << " E" << extrusion << "\n";
                retracted = false;
            }
            QPoint end = printPath(slice[i], extrusion, z);
            // o << "; Distance to next: "
            //   << std::to_string(distance(nextp, slice[i].last())) << "\n";
            if (distance(nextp, end) > 1.3) {
                // No retraction if very close
                extrusion -= retraction;
                out << "G1 F" << retractSpeed << " E"
                    << std::to_string(extrusion) << "\n";
                retracted = true;
            }
        }
        z += layerHeight;
    }
    endcode();
    out.flush();
}

void GCodeGenerator::startcode() {
    out << "M190 S" << bedTemp << "\n";
    out << "M109 S" << nozzleTemp << "\n";
    out << "M106 S0\n";
    out << "G21 ;metricvalues\n";
    out << "M201 X500.00 Y500.00 Z100.00 E5000.00 ;Setup machine max "
           "acceleration\n";
    out << "M203 X500.00 Y500.00 Z10.00 E50.00 ;Setup machine max feedrate\n";
    out << "M204 P500.00 R1000.00 T500.00 ;Setup Print/Retract/Travel "
           "acceleration\n";
    out << "M207 F5000 S5";
    out << "M205 X8.00 Y8.00 Z0.40 E5.00 ;Setup Jerk\n";
    out << "M220 S100 ;Reset Feedrate\n";
    out << "M221 S100 ;Reset Flowrate\n";
    out << "G28 ;Home\n";
    out << "G92 E0 ;Reset Extruder\n";
    out << "G1 Z2.0 F3000 ;Move Z Axis up\n";
    out << "G1 X10.1 Y20 Z0.28 F5000.0 ;Move to start position\n";
    out << "G1 X10.1 Y200.0 Z0.28 F1500.0 E15 ;Draw the first line\n";
    out << "G1 X10.4 Y200.0 Z0.28 F5000.0 ;Move to side a little\n";
    out << "G1 X10.4 Y20 Z0.28 F1500.0 E30 ;Draw the second line\n";
    out << "G92 E0 ;Reset Extruder\n";
    out << "G90 ; absolute positioning\n";
    out << "M82 ; extruder to absolute mode\n";
    out << "G1 F" << retractSpeed << " E-" << std::to_string(retraction)
        << "\n";
    // out << "G1 Z2.0 F3000 ;Move Z Axis up\n";
    // out << "G1 F2700 E-" << std::to_string(retraction) << "\n";
}

void GCodeGenerator::endcode() {
    // out << "G91 ; set relative pos mode \n"
    // << "G1 E-5 F600 retract filament \n"
    out << "M140 S0 ; turn off bed heating \n"
        << "M104 S0 ; turn off nozzle heating \n"
        << "G0 X5 F1800 ; home x-axis \n"
        << "G0 Y220 F1800; bring bed to the front \n "
        << "M106 S0; stop fan\n"
        << "M84; stop stepper motors";
}

void GCodeGenerator::gotoPoint(QPoint &p, double z) {
    if (z > 0) {
        out << "G0 F5000 X" << std::to_string(p.x() / 1000.0f) << " Y"
            << std::to_string(p.y() / 1000.0f) << " Z" << std::to_string(z)
            << "\n";
    } else {
        out << "G0 F5000 X" << std::to_string(p.x() / 1000.0f) << " Y"
            << std::to_string(p.y() / 1000.0f) << "\n";
    }
}

double GCodeGenerator::polyLength(const QPolygon &poly) {
    QPoint previous = poly[0];
    double len = 0;
    for (int i = 1; i < poly.size(); i++) {
        len += distance(previous, poly[i]);
        previous = poly[i];
    }
    return len;
}

double GCodeGenerator::printLine(QPoint &from, QPoint &to, double &extrusion,
                                 bool coast, double offset) {
    // extrusion += (4 * 0.2 * 0.95 * length) / (M_PI * 0.4);
    double length = distance(from, to);
    if (coast) {
        if (length <= 0.4) { // Don't extrude on last part
                             // Test: just do nothing
            out << "X" << std::to_string(to.x() / 1000.0f) << " Y"
                << std::to_string(to.y() / 1000.0f) << "\n";
        } else { // extrude only beginning of line
                 // Test: just go to almost the beginning
            extrusion += ((length - coastingDistance) * area * extrusion_scale);
            const double coastscale = length / (length - coastingDistance);
            double coastX;
            double coastY;
            coastX = ((to.x() - from.x()) / 1000.0f) / coastscale;
            coastY = ((to.y() - from.y()) / 1000.0f) / coastscale;
            // o << "; Coasting\n";
            out << "X" << std::to_string((from.x() / 1000.0f) + coastX) << " Y"
                << std::to_string((from.y() / 1000.0f) + coastY) << " E"
                << std::to_string(extrusion) << "\n";
            out << "G1 X" << std::to_string(to.x() / 1000.0f) << " Y"
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
        out << "X" << std::to_string(to.x() / 1000.0f) << " Y"
            << std::to_string(to.y() / 1000.0f) << " E"
            << std::to_string(extrusion) << "\n";
    }
    return length;
}

QPoint GCodeGenerator::printPath(QPolygon &poly, double &extrusion, double z) {
    QPoint p = poly[0], prevp(0, 0);
    double length = polyLength(poly);
    if (z < 0.4) {
        out << "G1 F" << wallSpeed * 0.75 << " ";
    } else {
        if (poly.size() > 2)
            out << "G1 F" << wallSpeed << " ";
        else
            out << "G1 F" << infillSpeed
                << " "; // Infill can be faster
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
        if (length <= coastingDistance && offset) {
            double l = printLine(p, pt, extrusion, true);
            length -= l;
        } else {
            double l = printLine(p, pt, extrusion, false, angle);
            length -= l;
        }
        prevp = pt;
        p = pt;
        if (i < (poly.size() - 1)) {
            out << "G1 ";
        }
    }
    if (poly.size() > 2) {
        out << "G1 ";
        printLine(p, poly[0], extrusion, true);
        return poly[0];
    } else {
        return p;
    }

    // o << "G1 F2700 E" << std::to_string(extrusion - retraction) << "\n";
}
