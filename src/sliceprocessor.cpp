#include "sliceprocessor.h"

#include <iostream>
#include <cmath>

SliceProcessor::SliceProcessor() {}

std::vector<QPolygon>
SliceProcessor::process(const std::vector<QPolygon> &paths) const {
    ClipperLib::Clipper clip;
    for (auto &path : paths) {
        ClipperLib::Path p;
        for (auto &point : path) {
            p << ClipperLib::IntPoint(point.x(), point.y());
        }
        clip.AddPath(p, ClipperLib::PolyType::ptClip, true);
    }
    ClipperLib::Paths clippedpaths;
    if (!clip.Execute(ClipperLib::ClipType::ctXor, clippedpaths)) {
		std::cout << "There was an error clipping paths" << std::endl;
	}
	// ClipperLib::CleanPolygons(clippedpaths);
	// TODO speedups

	ClipperLib::Paths processed;
	auto edge1 = getEdges(clippedpaths, true);
	auto edge2 = getEdges(edge1, false);
	auto edge3 = getEdges(edge2, false);
	processed.insert(processed.end(), edge1.begin(), edge1.end());
	processed.insert(processed.end(), edge2.begin(), edge2.end());
	processed.insert(processed.end(), edge3.begin(), edge3.end());

	auto infill = getInfill(edge3);
	processed.insert(processed.end(), infill.begin(), infill.end());

	std::vector<QPolygon> p;
    for (auto &path : processed) {
        QPolygon poly;
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

ClipperLib::Paths SliceProcessor::getInfill(const ClipperLib::Paths &edges,
										   const int direction) const {
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
	auto bounds = clip.GetBounds();

	long x = bounds.left, y = bounds.top;
	long xto = bounds.right, yto = bounds.bottom;
	if (x > bounds.right) {
		x = bounds.right;
		xto = bounds.left;
	}
	if (y > bounds.bottom) {	
		y = bounds.bottom;
		yto = bounds.bottom;
	}
    const long incr = std::sqrt((0.4 * 0.4) / 2) * 1000000 * 10;
	std::cout << "incr: " << incr << std::endl;
	const long lineoffset = 1000000000; // TODO get max bed size or calculate from bounds
	while (x < xto || y < yto) {
		ClipperLib::Path line;
		line << ClipperLib::IntPoint(x - lineoffset, y + lineoffset);
		line << ClipperLib::IntPoint(x + lineoffset, y - lineoffset);
		lines.push_back(line);
		line.clear();
		line << ClipperLib::IntPoint(xto - x + lineoffset, y + lineoffset);
		line << ClipperLib::IntPoint(xto - x - lineoffset, y - lineoffset);
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
