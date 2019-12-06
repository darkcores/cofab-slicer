#include "sliceprocessor.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <omp.h>

SliceProcessor::SliceProcessor(const QRect bounds) : bounds(bounds) {}

std::vector<std::vector<QPolygon>>
SliceProcessor::process(const std::vector<std::vector<QPolygon>> &paths) const {
    auto t1 = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<QPolygon>> processed(paths.size());
#pragma omp parallel for
    for (std::size_t i = 0; i < paths.size(); i++) {
        auto slice = processSlice(paths[i]);
        processed[i] = slice;
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    std::cout << "Processed in: " << duration << "ms " << std::endl;

    return processed;
}

std::vector<QPolygon>
SliceProcessor::processSlice(const std::vector<QPolygon> &paths) const {
    ClipperLib::Clipper clip;
    for (auto &path : paths) {
        ClipperLib::Path p;
        p.reserve(path.size());
        for (auto &point : path) {
            p << ClipperLib::IntPoint(point.x(), point.y());
        }
        clip.AddPath(p, ClipperLib::PolyType::ptClip, true);
    }
    ClipperLib::Paths clippedpaths;
    if (!clip.Execute(ClipperLib::ClipType::ctXor, clippedpaths)) {
        std::cout << "There was an error clipping paths" << std::endl;
    }

    ClipperLib::Paths processed;
    ClipperLib::CleanPolygons(clippedpaths);
    auto edges = getEdges(clippedpaths, true);
    processed.insert(processed.end(), edges.begin(), edges.end());
    for (int i = 1; i < num_walls; i++) {
        ClipperLib::CleanPolygons(edges);
        edges = getEdges(edges, false);
        processed.insert(processed.end(), edges.begin(), edges.end());
    }

    ClipperLib::CleanPolygons(edges);
    auto infill = getInfill(edges);
	optimizeInfill(infill);
    processed.insert(processed.end(), infill.begin(), infill.end());

    std::vector<QPolygon> p;
    p.reserve(processed.size());
    for (auto &path : processed) {
        QPolygon poly;
        poly.reserve(path.size());
        for (auto &point : path) {
            poly << QPoint(point.X, point.Y);
        }
        p.push_back(poly);
    }
    return p;
}

ClipperLib::Paths SliceProcessor::getEdges(const ClipperLib::Paths &paths,
                                           const bool first_path) const {
    ClipperLib::Paths newpaths;
    ClipperLib::ClipperOffset co;
    int offset = nozzle_offset;
    if (first_path) {
        offset /= 2;
    }
    co.AddPaths(paths, ClipperLib::JoinType::jtRound,
                ClipperLib::EndType::etClosedPolygon);
    co.Execute(newpaths, offset); // 1/2 nozzle * INT_SCALE
    return newpaths;
}

ClipperLib::Paths
SliceProcessor::getInfill(const ClipperLib::Paths &edges) const {
    ClipperLib::Paths contour;
    ClipperLib::ClipperOffset co;
    int offset = nozzle_offset;
    co.AddPaths(edges, ClipperLib::JoinType::jtRound,
                ClipperLib::EndType::etClosedPolygon);
    co.Execute(contour, offset); // 1/2 nozzle * INT_SCALE

    ClipperLib::Paths lines;

    ClipperLib::PolyTree contour_tree;
    ClipperLib::Clipper clip;
    clip.AddPaths(contour, ClipperLib::PolyType::ptClip, true);
    clip.Execute(ClipperLib::ClipType::ctXor, contour_tree);

    long x = bounds.left(), y = bounds.top();
    const long xfrom = bounds.left(), yfrom = bounds.top();
    const long xto = bounds.right(), yto = bounds.bottom();

    const long incr = std::sqrt((0.4 * 0.4) / 2) * 10000 * infill_offset;
    lines.resize(((xto - xfrom) / incr) + ((yto - yfrom) / incr));
    // std::cout << "incr: " << incr << std::endl;
    ClipperLib::Path line(2);
    while (x < 2 * xto || y < 2 * yto) {
        line.clear();
        line << ClipperLib::IntPoint(xfrom, y);
        line << ClipperLib::IntPoint(x, yfrom);
        lines.push_back(line);
        line.clear();
        line << ClipperLib::IntPoint(xto, y);
        line << ClipperLib::IntPoint(xto - (x - xfrom), yfrom);
        lines.push_back(line);
        x += incr;
        y += incr;
        // std::cout << x << " - " << y << std::endl;
    }

    ClipperLib::PolyTree lines_tree;
    clip.Clear();
    clip.AddPaths(lines, ClipperLib::PolyType::ptSubject, false);
    clip.AddPaths(contour, ClipperLib::PolyType::ptClip, true);
    clip.Execute(ClipperLib::ClipType::ctIntersection, lines_tree);

    ClipperLib::OpenPathsFromPolyTree(lines_tree, lines);
    return lines;
}

void SliceProcessor::optimizeInfill(ClipperLib::Paths &edges) const {
    if (edges.size() < 2) {
        return;
    }

    long distance;
    ClipperLib::IntPoint lastPoint = edges[0][1];
    distance = std::pow(lastPoint.X - edges[1][0].X, 2);
    distance += std::pow(lastPoint.Y - edges[1][0].Y, 2);
    for (std::size_t i = 1; i < edges.size() - 1; i++) {
        for (std::size_t j = i + 1; j < edges.size(); j++) {
            bool swap = false;
            bool invert = false;
            long newdist;
            // Calculate for point 1
            newdist = std::pow(lastPoint.X - edges[j][0].X, 2);
            newdist += std::pow(lastPoint.Y - edges[j][0].Y, 2);
            swap = newdist < distance;

            // Calculate for point 2 (inverted line)
            long newdist2 = std::pow(lastPoint.X - edges[j][1].X, 2);
            newdist2 += std::pow(lastPoint.Y - edges[j][1].Y, 2);
            if (newdist2 < distance && newdist2 < newdist) {
                swap = true;
                invert = true;
				newdist = newdist2;
            }

            // Swap if closer
            if (swap) {
                if (invert) {
					ClipperLib::Path newpath;
					newpath.push_back(edges[j][1]);
					newpath.push_back(edges[j][0]);
					edges[j] = edges[i];
					edges[i] = newpath;
					distance = newdist;
                } else {
					auto tmp = edges[j];
					edges[j] = edges[i];
					edges[i] = tmp;
					distance = newdist;
                }
            }
        }
        lastPoint = edges[i][1];
        distance = std::pow(lastPoint.X - edges[i + 1][0].X, 2);
        distance += std::pow(lastPoint.Y - edges[i + 1][0].Y, 2);
    }
}
