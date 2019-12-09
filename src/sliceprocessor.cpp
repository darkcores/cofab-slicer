#include "sliceprocessor.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <omp.h>

SliceProcessor::SliceProcessor(const QRect bounds) : bounds(bounds) {}

std::vector<std::vector<QPolygon>>
SliceProcessor::process(const std::vector<std::vector<QPolygon>> &paths) {
    auto t1 = std::chrono::high_resolution_clock::now();

    // Clip polygons
    clipped_slices.resize(paths.size());
#pragma omp parallel for
    for (std::size_t i = 0; i < paths.size(); i++) {
        auto slice = clipPoly(paths[i]);
        clipped_slices[i] = slice;
    }

    // Calculate paths
    std::vector<std::vector<QPolygon>> processed(clipped_slices.size());
#pragma omp parallel for
    for (std::size_t i = 0; i < clipped_slices.size(); i++) {
        auto slice = processSlice(clipped_slices[i], i);
        processed[i] = slice;
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    std::cout << "Processed in: " << duration << "ms " << std::endl;

    return processed;
}

ClipperLib::Paths
SliceProcessor::clipPoly(const std::vector<QPolygon> &poly) const {
    ClipperLib::Clipper clip;
    for (auto &path : poly) {
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
    ClipperLib::CleanPolygons(clippedpaths);
    return clippedpaths;
}

std::vector<QPolygon>
SliceProcessor::processSlice(const ClipperLib::Paths &clippedpaths,
                             const std::size_t idx) const {

    ClipperLib::Paths processed;
    auto edges = getEdges(clippedpaths, true);
    processed.insert(processed.end(), edges.begin(), edges.end());
    for (unsigned int i = 1; i < num_walls; i++) {
        ClipperLib::CleanPolygons(edges);
        edges = getEdges(edges, false);
        processed.insert(processed.end(), edges.begin(), edges.end());
    }
    optimizeEdges(processed, idx);
    ClipperLib::CleanPolygons(edges);

    auto dense = getDenseArea(edges, idx);
    // processed = dense;
    auto denseInfill = getDenseInfill(dense, (idx % 2) == 0);
    optimizeInfill(denseInfill);
    processed.insert(processed.end(), denseInfill.begin(), denseInfill.end());

    ClipperLib::Paths infillArea;
    ClipperLib::Clipper infillClip;
    infillClip.AddPaths(edges, ClipperLib::PolyType::ptSubject, true);
    infillClip.AddPaths(dense, ClipperLib::PolyType::ptClip, true);
    infillClip.Execute(ClipperLib::ClipType::ctDifference, infillArea);

    auto infill = getInfill(infillArea);
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

    const long incr = std::sqrt(pow(2 * (0.4 * 1000), 2)) / 2 * infill_offset;
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

ClipperLib::Paths SliceProcessor::getDenseInfill(const ClipperLib::Paths &edges,
                                                 const bool direction) const {
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

    const long incr = std::sqrt(pow(2 * (0.4 * 1000), 2)) / 2;
    lines.resize(((xto - xfrom) / incr) + ((yto - yfrom) / incr));
    // std::cout << "incr: " << incr << std::endl;
    ClipperLib::Path line(2);
    while (x < 2 * xto || y < 2 * yto) {
        line.clear();
        if (direction) {
            line << ClipperLib::IntPoint(xfrom, y);
            line << ClipperLib::IntPoint(x, yfrom);
        } else {
            line << ClipperLib::IntPoint(xto, y);
            line << ClipperLib::IntPoint(xto - (x - xfrom), yfrom);
        }
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

void SliceProcessor::optimizeEdges(ClipperLib::Paths &edges,
                                   std::size_t idx) const {
    if (edges.size() < 2)
        return;

    auto checkpt = [=](ClipperLib::IntPoint pt) -> bool {
        ClipperLib::Clipper clippt;
        ClipperLib::Path p;
        ClipperLib::Paths ps;
        // p << pt;
        p << ClipperLib::IntPoint(pt.X - 2, pt.Y - 2);
        p << ClipperLib::IntPoint(pt.X + 2, pt.Y - 2);
        p << ClipperLib::IntPoint(pt.X + 2, pt.Y + 2);
        p << ClipperLib::IntPoint(pt.X - 2, pt.Y + 2);
        clippt.AddPath(p, ClipperLib::PolyType::ptSubject, true);
        clippt.AddPaths(clipped_slices[idx - 1], ClipperLib::PolyType::ptClip,
                        true);
        clippt.Execute(ClipperLib::ClipType::ctIntersection, ps);
        return ps.size() > 0;
    };

    // Check if starting on previous layer -> changing start points
    if (idx > 0) {
        int good = 0, bad = 0, moved = 0;
        for (std::size_t i = 0; i < edges.size(); i++) {
            if (checkpt(edges[i][0])) {
                good++;
                continue;
            } else {
                bad++;
            }
			for (std::size_t j = 1; j < edges[i].size(); j++) {
				if (checkpt(edges[i][j])) {
					ClipperLib::Path tmp(edges[i].begin(), edges[i].begin() + j);
					std::move(edges[i].begin() + j, edges[i].end(), edges[i].begin());
					std::move(tmp.begin(), tmp.end(), edges[i].end() - j);
					moved++;
					break;
				}
			}
        }
        if (bad > 0) {
#pragma omp critical
            {
                std::cout << "Layer: " << idx << " - "
                          << "Good points: " << good << " / bad points: " << bad
                          << " (moved: " << moved << ")" << std::endl;
            }
        }
    }

    long distance;
    ClipperLib::IntPoint lastPoint = edges[0][1];
    distance = std::pow(lastPoint.X - edges[1][0].X, 2);
    distance += std::pow(lastPoint.Y - edges[1][0].Y, 2);
    for (std::size_t i = 1; i < edges.size() - 1; i++) {
        for (std::size_t j = i + 1; j < edges.size(); j++) {
            long newdist = std::pow(lastPoint.X - edges[j][0].X, 2);
            newdist += std::pow(lastPoint.Y - edges[j][0].Y, 2);
            if (newdist < distance) {
                auto tmp = edges[j];
                edges[j] = edges[i];
                edges[i] = tmp;
                distance = newdist;
            }
        }
    }
}

void SliceProcessor::optimizeInfill(ClipperLib::Paths &edges) const {
    if (edges.size() < 2)
        return;

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

ClipperLib::Paths
SliceProcessor::getDenseArea(const ClipperLib::Paths &infillArea,
                             const std::size_t idx) const {
    ClipperLib::Paths paths;
    ClipperLib::Clipper unionclip;

    if ((idx < num_rf) || (idx + num_rf) >= clipped_slices.size()) {
        // Always floor or roof.
        // std::cout << "idx: " << idx << " full fill" << std::endl;
        return infillArea;
    } else {
        // Add bounding rect subject
        ClipperLib::Path clipbounds;
        clipbounds << ClipperLib::IntPoint(bounds.left(), bounds.top());
        clipbounds << ClipperLib::IntPoint(bounds.right(), bounds.top());
        clipbounds << ClipperLib::IntPoint(bounds.right(), bounds.bottom());
        clipbounds << ClipperLib::IntPoint(bounds.left(), bounds.bottom());
        // unionclip.AddPath(clipbounds, ClipperLib::PolyType::ptSubject, true);
        for (unsigned i = (idx - num_rf); i < (idx + num_rf + 1); i++) {
            if (i != idx) {
                ClipperLib::Clipper diffclip;
                diffclip.AddPath(clipbounds, ClipperLib::PolyType::ptSubject,
                                 true);
                diffclip.AddPaths(clipped_slices[i],
                                  ClipperLib::PolyType::ptClip, true);
                ClipperLib::Paths p;
                diffclip.Execute(ClipperLib::ClipType::ctDifference, p);
                unionclip.AddPaths(p, ClipperLib::PolyType::ptClip, true);
            }
        }
    }
    unionclip.Execute(ClipperLib::ClipType::ctUnion, paths,
                      ClipperLib::PolyFillType::pftPositive,
                      ClipperLib::PolyFillType::pftPositive);

    // Offset paths for walls or other thing to support on.
    ClipperLib::ClipperOffset co;
    co.AddPaths(paths, ClipperLib::JoinType::jtRound,
                ClipperLib::EndType::etClosedPolygon);
    co.Execute(paths, -(nozzle_offset * num_walls)); // 1/2 nozzle * INT_SCALE

    // return paths;

    ClipperLib::Paths clippedpaths;
    ClipperLib::Clipper diffclip;
    diffclip.AddPaths(infillArea, ClipperLib::PolyType::ptSubject, true);
    diffclip.AddPaths(paths, ClipperLib::PolyType::ptClip, true);
    diffclip.Execute(ClipperLib::ClipType::ctIntersection, clippedpaths);

    return clippedpaths;
}
