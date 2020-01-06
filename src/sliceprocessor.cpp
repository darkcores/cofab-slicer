#include "sliceprocessor.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <omp.h>

SliceProcessor::SliceProcessor(const QRect bounds) : bounds(bounds) {}

void SliceProcessor::setNozzleWidth(double w) {
    nozzleWidth = w;
    wall_width = round(((nozzleWidth * 1.05) - (layerHeight * (1 - (M_PI / 4)))) * 1000);
    nozzle_offset = -wall_width;
	std::cout << "Width: " << wall_width << std::endl;
	std::cout << nozzleWidth << " - " << layerHeight << std::endl;
}

void SliceProcessor::setLayerHeight(double h) {
    layerHeight = h;
    wall_width = round(((nozzleWidth * 1.05) - (layerHeight * (1 - (M_PI / 4)))) * 1000);
    nozzle_offset = -wall_width;
	std::cout << "Width: " << wall_width << std::endl;
}

ClipperLib::Paths
SliceProcessor::getInfillSupport(const ClipperLib::Paths &edges) const {
    ClipperLib::Paths contour;
    ClipperLib::ClipperOffset co;
    int offset = nozzle_offset / 2;
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

    const long incr = std::sqrt(pow(2 * wall_width, 2) / 2) * infill_offset/3;
    lines.reserve(((xto - xfrom) / incr) + ((yto - yfrom) / incr));
    // std::cout << "incr: " << incr << std::endl;
    ClipperLib::Path line(2);
    while (x < xto || y < yto) {
        line.clear();
        line << ClipperLib::IntPoint(xfrom, y);
        line << ClipperLib::IntPoint(xto, y);
        lines.push_back(line);
        line.clear();
        line << ClipperLib::IntPoint(x, yto);
        line << ClipperLib::IntPoint(x, yfrom);
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

bool SliceProcessor::firstLayerOffSupport(const ClipperLib::Paths &i, const ClipperLib::Paths &j, const int offset ) const{
  ClipperLib::Clipper differenceLayers;
  ClipperLib::Paths original;
  differenceLayers.AddPaths(j, ClipperLib::PolyType::ptSubject, true);
  differenceLayers.AddPaths(i, ClipperLib::ptClip, true);
  differenceLayers.Execute(ClipperLib::ClipType::ctDifference, original);

  ClipperLib::CleanPolygons(original);
  ClipperLib::Paths smaller = getOffsetEdges(original, offset);

  if(smaller.size()>0){
    return true;
  }
  return false;
}

void SliceProcessor::addSupport(std::vector<std::vector<QPolygon>> &processed) {
    // get slice difference with the last one
    // start from top of the model to bottom
    if (clipped_slices.size() < 2) {
        return;
    }
    std::vector<ClipperLib::Paths> original_slices(clipped_slices);


    auto j = clipped_slices.size() - 1;
    for (auto i = clipped_slices.size() - 2; i-- > 0;) {
        // std::cout << "layer " << i << ": "<< clipped_slices.at(i) <<
        // std::endl; difference between layers
        int offset = -nozzleWidth * 1000;
        ClipperLib::Clipper differenceLayers;
        ClipperLib::Paths diff;

        differenceLayers.AddPaths(clipped_slices[j],
                                  ClipperLib::PolyType::ptSubject, true);
        differenceLayers.AddPaths(clipped_slices[i], ClipperLib::ptClip, true);
        differenceLayers.Execute(ClipperLib::ClipType::ctDifference, diff);
        // offset the edges: because the vertices of a triangle with 90°,45°,
        // 45° angles are the same length
        ClipperLib::CleanPolygons(diff);
        ClipperLib::Paths smaller = getOffsetEdges(diff, offset);

        //make sure it isn't the first layer after the structure that needs support
        if (diff.size() > 0 && smaller.size() > 0) {
            // add support structure (smaller)
            // union with current layer and the smaller section
            ClipperLib::Clipper unionclip;
            ClipperLib::Clipper threeunionclip;
            ClipperLib::Clipper differenceclip;
            ClipperLib::Paths output;
            unionclip.AddPaths(diff, ClipperLib::PolyType::ptSubject, true);
            unionclip.AddPaths(clipped_slices[i], ClipperLib::PolyType::ptClip,
                               true);
            unionclip.Execute(ClipperLib::ClipType::ctUnion, clipped_slices[i],
                              ClipperLib::PolyFillType::pftNonZero,
                              ClipperLib::PolyFillType::pftNonZero);
            //check if last or first layer
            if(i+1< original_slices.size() && i > 0){
              threeunionclip.AddPaths(original_slices[i], ClipperLib::PolyType::ptSubject, true);
              threeunionclip.AddPaths(original_slices[i+1], ClipperLib::PolyType::ptClip,
                                 true);
              threeunionclip.AddPaths(original_slices[i-1], ClipperLib::PolyType::ptClip,
                                 true);
              threeunionclip.Execute(ClipperLib::ClipType::ctUnion, output);


              //convert to qpolygon
              differenceclip.AddPaths(smaller,
                                        ClipperLib::PolyType::ptSubject, true);
              differenceclip.AddPaths(output, ClipperLib::ptClip, true);
              differenceclip.Execute(ClipperLib::ClipType::ctDifference, smaller);
            }

            //------------

            auto grid = getInfillSupport(smaller);
            optimizeInfill(grid);

            //std::cout<<grid<<std::endl;
            grid.insert(grid.end(), smaller.begin(), smaller.end());
            std::vector<QPolygon> p;
            p.reserve(grid.size());
            for (auto &path : grid) {
                QPolygon poly;
                poly.reserve(path.size());
                for (auto &point : path) {
                    poly << QPoint(point.X, point.Y);
                }
                p.push_back(poly);
            }
            //update the layer
            processed[i].insert(processed[i].end(), p.begin(), p.end());

        }
        j--;
    }
}

ClipperLib::Paths SliceProcessor::getGrid(const int gridSpace,
                                          const ClipperLib::Paths area) {

    ClipperLib::Paths grid;
    ClipperLib::Clipper c;
    for (auto &path : area) {
        int minX = 0, maxX = 0;
        int minY = 0, maxY = 0;
        for (auto point : path) {
            if (point.X < minX)
                minX = point.X;
            if (point.X > maxX)
                maxX = point.X;
            if (point.Y < minY)
                minY = point.Y;
            if (point.Y > maxY)
                maxY = point.Y;
        }

        // horizontal lines
        for (int i = 0; i <= (maxX - minX) / gridSpace; i++) {
            ClipperLib::Path horizontaline = ClipperLib::Path(2);
            horizontaline[0] =
                ClipperLib::IntPoint(minX, maxX - minX + i * gridSpace);
            horizontaline[1] =
                ClipperLib::IntPoint(maxX, maxX - minX + i * gridSpace);
            grid.insert(grid.end(), horizontaline);
        }

        // vertical
        for (int j = 0; j <= (maxY - minY) / gridSpace; j++) {
            ClipperLib::Path verticalline = ClipperLib::Path(2);
            verticalline[0] =
                ClipperLib::IntPoint(maxY - minY + j * gridSpace, minY);
            verticalline[1] =
                ClipperLib::IntPoint(maxY - minY + j * gridSpace, maxY);
            grid.insert(grid.end(), verticalline);
        }
    }
    return grid;
}

ClipperLib::Paths SliceProcessor::getOffsetEdges(const ClipperLib::Paths &paths,
                                                 const int offset) const {
    ClipperLib::Paths newpaths;
    ClipperLib::ClipperOffset co;
    co.AddPaths(paths, ClipperLib::JoinType::jtRound,
                ClipperLib::EndType::etClosedPolygon);
    co.Execute(newpaths, offset);
    return newpaths;
}

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

	if (support) {
		std::cout << "add support" << std::endl;
		// addSupport
		addSupport(processed);
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
    ClipperLib::CleanPolygons(edges);
    processed.insert(processed.end(), edges.begin(), edges.end());
    for (unsigned int i = 1; i < num_walls; i++) {
        edges = getEdges(edges, false);
        ClipperLib::CleanPolygons(edges);
        processed.insert(processed.end(), edges.begin(), edges.end());
    }
    optimizeEdges(processed, idx);
    edges = getEdges(edges, true);
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
    int offset = nozzle_offset / 2;
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

    const long incr = std::sqrt(pow(2 * wall_width, 2) / 2) * infill_offset;
    lines.reserve(((xto - xfrom) / incr) + ((yto - yfrom) / incr));
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
    int offset = nozzle_offset / 2;
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

    const long incr =
        std::sqrt(pow(2 * wall_width, 2) / 2); // lines closer to eachother
    lines.reserve(((xto - xfrom) / incr) + ((yto - yfrom) / incr));
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
        p << ClipperLib::IntPoint(pt.X - 1, pt.Y - 1);
        p << ClipperLib::IntPoint(pt.X + 1, pt.Y - 1);
        p << ClipperLib::IntPoint(pt.X + 1, pt.Y + 1);
        p << ClipperLib::IntPoint(pt.X - 1, pt.Y + 1);
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
            if (edges[i].size() < 3) {
                std::cout << "Non polygon in structure but continueing\n";
                std::cout << "This is probably a bug if you see this"
                          << std::endl;
                continue;
            }
            if (checkpt(edges[i][0])) {
                good++;
                continue;
            } else {
                bad++;
            }
            int pton = 0;
            for (std::size_t j = 1; j < edges[i].size(); j++) {
                if (checkpt(edges[i][j])) {
                    if (pton > 2) {
                        ClipperLib::Path tmp(edges[i].begin(),
                                             edges[i].begin() + j);
                        std::move(edges[i].begin() + j, edges[i].end(),
                                  edges[i].begin());
                        std::move(tmp.begin(), tmp.end(), edges[i].end() - j);
                        moved++;
                        break;
                    } else {
                        pton++;
                    }
                } else {
					pton = 0;
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
    ClipperLib::IntPoint lastPoint = edges[0][0];
    for (std::size_t i = 1; i < edges.size() - 1; i++) {
		if (edges[i].size() < 3) {
			// Bad if this happens
			continue;
		}
        distance = std::pow(lastPoint.X - edges[i][0].X, 2);
        distance += std::pow(lastPoint.Y - edges[i][0].Y, 2);
        for (std::size_t j = i + 1; j < edges.size(); j++) {
			if (edges[j].size() < 3) {
				// Bad if this happens
				continue;
			}
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
        for (std::size_t j = i; j < edges.size(); j++) {
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
