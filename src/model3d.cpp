#include "model3d.h"

#include <cassert>
#include <chrono>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <QPoint>

Model3D::Model3D(const std::string &filename) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        filename, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                      aiProcess_ImproveCacheLocality | aiProcess_SortByPType);

    if (!scene) {
        // Error loading file.
        std::cerr << "Could not load file" << std::endl;
        std::cerr << importer.GetErrorString() << std::endl;
        return;
    }

    const aiMesh *mesh = scene->mMeshes[0];

    // std::cout << "Loaded " << scene->mNumMeshes << " meshes" << std::endl;
    std::cout << "has faces: " << mesh->HasFaces()
              << " faces: " << mesh->mNumFaces << std::endl;

    for (std::size_t i = 0; i < mesh->mNumVertices; i++) {
        auto ov = mesh->mVertices[i];
		if (ov.z < z_bottom) 
			z_bottom = ov.z;
		if (ov.z > z_top) 
			z_top = ov.z;
		if (ov.x < x_left)
			x_left = ov.x;
		if (ov.x > x_right)
			x_right = ov.x;
		if (ov.y < y_left)
			y_left = ov.y;
		if (ov.y > y_right)
			y_right = ov.y;
        QVector3D v(ov.x, ov.y, ov.z);
        vertices.push_back(v);
    }

	// Move points to positive values
	float x_offset = 0, y_offset = 0;
	if (x_left < 0) {
		x_offset = fabs(x_left);
		x_left = 0;
		x_right += x_offset;
	}
	if (y_left < 0) {
		y_offset = fabs(y_left);
		y_left = 0;
		y_right += y_offset;
	}
	if (x_offset != 0 || y_offset != 0) { 
		for (auto &v :vertices ) {
			v += QVector3D(x_offset, y_offset, 0);
		}
	}

    for (std::size_t i = 0; i < mesh->mNumFaces; i++) {
        auto face = mesh->mFaces[i];
        assert(face.mNumIndices == 3);
        std::array<std::size_t, 3> f = {face.mIndices[0], face.mIndices[1],
                                        face.mIndices[2]};
        faces.push_back(f);
    }
    layerHeight = 0.2;
    currentLayer = z_bottom + 0.1;
}

std::vector<std::vector<QPolygon>> Model3D::getSlices() {
    auto t1 = std::chrono::high_resolution_clock::now();

	std::vector<std::vector<QPolygon>> slices;
	while (currentLayer < z_top) {
		auto slice = getSlice();
		slices.push_back(slice);
		currentLayer += layerHeight;
	}
	
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    std::cout << "Sliced in: " << duration << "ms " << std::endl;
	
	return slices;
}

std::vector<QPolygon> Model3D::getSlice() const {
    auto lines = getLines();
    // std::cout << lines.size() << " lines" << std::endl;

    std::vector<QPolygon> polygons;
    std::vector<bool> used(lines.size());

    auto done = [&used]() -> bool {
        for (auto u : used) {
            if (!u)
                return false;
        }
        return true;
    };

    std::size_t firstpoint = 0;

    auto findNext = [&used, &lines, &firstpoint](const QPoint value) -> QPoint {
        for (std::size_t i = firstpoint; i < lines.size(); i++) {
            if (used[i])
                continue;
            if (lines[i].p1() == value) {
                used[i] = true;
                return lines[i].p2();
            }
            if (lines[i].p2() == value) {
                used[i] = true;
                return lines[i].p1();
            }
        }
        return QPoint(-9999, -9999);
    };

    auto getFirst = [&used, &lines, &firstpoint]() -> QLine {
        for (std::size_t i = firstpoint; i < used.size(); i++) {
            if (!used[i]) {
                firstpoint = i + 1;
                used[i] = true;
                return lines[i];
            }
        }
        return QLine(-9999, -9999, -9999, -9999);
    };

    while (!done()) {
        QPolygon poly;
        auto start = getFirst();
        poly << start.p1() << start.p2();
        QPoint next = findNext(start.p2());
        while (next != QPoint(-9999, -9999)) {
            poly << next;
            next = findNext(next);
        }
        polygons.push_back(poly);
    }

    // std::cout << polygons.size() << " Polygons" << std::endl;
    return polygons;
}

std::vector<QLine> Model3D::getLines() const {
    std::vector<QLine> lines;

    auto layer = currentLayer;
    bool loop = true;

    while (loop) {
		std::size_t found_ctr = 0, unique_ctr = 0;
        for (auto &f : faces) {
            if (min_z(f) <= layer && max_z(f) >= layer) {
                // Calculate line
                // const auto face = getFace(f);

                // Get the number of faces above / under line
                std::vector<std::size_t> above, on, under;
                for (auto vertex : f) {
                    if (vertices[vertex].z() < layer) {
                        under.push_back(vertex);
                    } else if (vertices[vertex].z() > layer) {
                        above.push_back(vertex);
                    } else {
                        on.push_back(vertex);
                    }
                }

                /*
                 * This part could really be optimized, but for now just
                 * trying to get it working.
                 */

                // Decide what to do based on triangle
                if (above.size() == 3 || under.size() == 3) {
                    // Not intersecting
                    continue;
                } else if (above.size() == 1 && under.size() == 2) {
                    // calculate 2 lines
                    QPoint v1 = getZPoint(above[0], under[0]);
                    QPoint v2 = getZPoint(above[0], under[1]);
                    if (v1 == v2)
                        continue;
                    QLine l(v1, v2);
                    lines.push_back(l);
                } else if (above.size() == 2 && under.size() == 1) {
                    // calculate 2 lines
                    QPoint v1 = getZPoint(above[0], under[0]);
                    QPoint v2 = getZPoint(above[1], under[0]);
                    if (v1 == v2)
                        continue;
                    QLine l(v1, v2);
                    lines.push_back(l);
                } else if (on.size() == 1) {
                    if (above.size() == 2 || under.size() == 2) {
                        continue;
                    } else {
                        // Calculate 1 point
                        QPoint v1 = getZPoint(above[0], under[0]);
                        QPointF v2_tmp(vertices[on[0]].x(),
                                       vertices[on[0]].y());
                        QPoint v2 = (v2_tmp * INT_SCALE).toPoint();
                        if (v1 == v2)
                            continue;
                        QLine l(v1, v2);
                        lines.push_back(l);
                    }
                } else if (on.size() == 2) {
                    std::cout << "2 on a line" << std::endl;
                    // Keep 2 lines
                    QPointF v1_tmp(vertices[on[0]].x(), vertices[on[0]].y());
                    QPointF v2_tmp(vertices[on[1]].x(), vertices[on[1]].y());
                    QPoint v1 = (v1_tmp * INT_SCALE).toPoint();
                    QPoint v2 = (v2_tmp * INT_SCALE).toPoint();
                    if (v1 == v2)
                        continue;
                    QLine l(v1, v2);
                    found_ctr += 1;
                    bool found = false;
                    for (auto &x : lines) {
                        if (x == l) {
                            std::cout << "Not adding line in 2 on" << std::endl;
                            found = true;
                            break;
                        }
                    }
                    if (found)
                        continue;
                    unique_ctr += 1;
                    std::cout << "(" << v1.x() << "," << v1.y() << ")("
                              << v2.x() << "," << v2.y() << ")" << std::endl;
                    lines.push_back(l);
                }
            }
        }

        if (found_ctr != (2 * unique_ctr)) {
            layer -= 2e-14; // Move down a little bit so we don't have plane issues.
            lines.clear();
        } else {
            loop = false;
        }
    }

    return lines;
}
