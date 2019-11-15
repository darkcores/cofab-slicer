#include "model3d.h"

#include <cassert>
#include <chrono>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

Model3D::Model3D(const std::string &filename) {
    layerHeight = 0.2;
    currentLayer = 40.1;

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        filename, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                      aiProcess_ImproveCacheLocality | aiProcess_SortByPType );

    if (!scene) {
        // Error loading file.
        std::cerr << "Could not load file" << std::endl;
        std::cerr << importer.GetErrorString() << std::endl;
        return;
    }

    const aiMesh *mesh = scene->mMeshes[0];

    std::cout << "Loaded " << scene->mNumMeshes << " meshes" << std::endl;
    std::cout << "has faces: " << mesh->HasFaces()
              << " faces: " << mesh->mNumFaces << std::endl;

    for (std::size_t i = 0; i < mesh->mNumVertices; i++) {
        auto ov = mesh->mVertices[i];
        QVector3D v(ov.x, ov.y, ov.z);
        vertices.push_back(v);
    }

    for (std::size_t i = 0; i < mesh->mNumFaces; i++) {
        auto face = mesh->mFaces[i];
        assert(face.mNumIndices == 3);
        std::array<std::size_t, 3> f = {face.mIndices[0], face.mIndices[1],
                                        face.mIndices[2]};
        faces.push_back(f);
    }
}

std::vector<QPolygonF> Model3D::getSlice() const {
    std::vector<QLineF> lines;
    auto t1 = std::chrono::high_resolution_clock::now();

    for (auto &f : faces) {
        if (min_z(f) <= currentLayer && max_z(f) >= currentLayer) {
            // Calculate line
            // const auto face = getFace(f);

            // Get the number of faces above / under line
            std::vector<std::size_t> above, on, under;
            for (auto vertex : f) {
                if (vertices[vertex].z() < currentLayer) {
                    under.push_back(vertex);
                } else if (vertices[vertex].z() > currentLayer) {
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
                QPointF v1 = getZPoint(above[0], under[0]);
                QPointF v2 = getZPoint(above[0], under[1]);
				if (v1 == v2)
					continue;
                QLineF l(v1, v2);
                lines.push_back(l);
            } else if (above.size() == 2 && under.size() == 1) {
                // calculate 2 lines
                QPointF v1 = getZPoint(above[0], under[0]);
                QPointF v2 = getZPoint(above[1], under[0]);
				if (v1 == v2)
					continue;
                QLineF l(v1, v2);
                lines.push_back(l);
            } else if (on.size() == 1) {
                if (above.size() == 2 || under.size() == 2) {
                    continue;
                } else {
                    // Calculate 1 point
                    QPointF v1 = getZPoint(above[0], under[0]);
                    QPointF v2(vertices[on[0]].x(), vertices[on[0]].y());
					if (v1 == v2)
						continue;
                    QLineF l(v1, v2);
                    lines.push_back(l);
                }
            } else if (on.size() == 2) {
                if (on.size() == 3) {
                    std::cout << "3 on a line flat surface" << std::endl;
                    std::cout << "Not implemented" << std::endl;
                    continue;
                }
                std::cout << "2 on a line" << std::endl;
                // Keep 2 lines
                QPointF v1(vertices[on[0]].x(), vertices[on[0]].y());
                QPointF v2(vertices[on[1]].x(), vertices[on[1]].y());
				if (v1 == v2)
					continue;
                QLineF l(v1, v2);
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
                std::cout << "(" << v1.x() << "," << v1.y() << ")(" << v2.x()
                          << "," << v2.y() << ")" << std::endl;
                lines.push_back(l);
            }
        }
    }

    std::cout << lines.size() << " lines" << std::endl;

    std::vector<QPolygonF> polygons;
    std::vector<bool> used(lines.size());

    auto done = [&used]() -> bool {
        for (auto u : used) {
            if (!u)
                return false;
        }
        return true;
    };

    std::size_t firstpoint = 0;

    auto findNext = [&used, &lines,
                     &firstpoint](const QPointF value) -> QPointF {
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
        return QPointF(-9999, -9999);
    };

    auto getFirst = [&used, &lines, &firstpoint]() -> QLineF {
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
        QPolygonF poly;
        auto start = getFirst();
        poly << start.p1() << start.p2();
        QPointF next = findNext(start.p2());
        while (next != QPointF(-9999, -9999)) {
            poly << next;
            next = findNext(next);
        }
		polygons.push_back(poly);
    }

    for (auto &p : polygons) {
        std::cout << "Polygon size: " << p.size() << std::endl;
        if (p.size() < 10) {
			for (auto &pt : p) {
				std::cout << "(" << pt.x() << "," << pt.y() << ")" << std::endl;
			}
        }
        for (auto &pt : p) {
            pt *= 15;
            pt += QPointF(400, 200);
        }
    }

    std::cout << polygons.size() << " Polygons" << std::endl;

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    std::cout << "Slice in: " << duration << "ms " << std::endl;

    return polygons;
}
