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
    currentLayer = 0.1;

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        filename, aiProcess_CalcTangentSpace | aiProcess_Triangulate |
                      aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

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

std::vector<QLineF> Model3D::getSlice() const {
    std::vector<QLineF> points;
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
                } else if (vertices[vertex].z() == currentLayer) {
                    on.push_back(vertex);
                } else {
                    above.push_back(vertex);
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
                // calculate 2 points
                QPointF v1 = getZPoint(above[0], under[0]);
                QPointF v2 = getZPoint(above[0], under[1]);
				QLineF l(v1, v2);
                points.push_back(l);
            } else if (above.size() == 2 && under.size() == 1) {
                // calculate 2 points
                QPointF v1 = getZPoint(above[0], under[0]);
                QPointF v2 = getZPoint(above[1], under[0]);
				QLineF l(v1, v2);
                points.push_back(l);
            } else if (on.size() == 1) {
                if (above.size() == 2 || under.size() == 2) {
                    continue;
                } else {
                    // Calculate 1 point
                    QPointF v1 = getZPoint(above[0], under[0]);
					QPointF v2(vertices[on[1]].x(), vertices[on[1]].y());
                    QLineF l(v1, v2);
                    points.push_back(l);
                }
            } else if (on.size() == 2) {
                // Keep 2 points
				QPointF v1(vertices[on[0]].x(), vertices[on[0]].y());
				QPointF v2(vertices[on[1]].x(), vertices[on[1]].y());
				QLineF l(v1, v2);
                points.push_back(l);
            }
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    std::cout << "Slice in: " << duration << "ms " << std::endl;
    std::cout << points.size() << " lines" << std::endl;

    for (auto &l : points) {
        // l *= 10;
		l.setP1(l.p1() * 10);
		l.setP2(l.p2() * 10);
        l.translate(300, 100);
        /* std::cout << "(" << l.x1 << "," << l.y1 << "),(" << l.x2 << "," <<
           l.y2
                   << ")" << std::endl; */
    }

    return points;
}
