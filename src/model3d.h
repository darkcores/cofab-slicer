#ifndef MODEL3D_H
#define MODEL3D_H

#include <algorithm>
#include <string>
#include <vector>

/**
 * Handle loading 3D models and calculating 2D intersections.
 */
class Model3D {
    struct Vertex {
        double x, y, z;
    };

  public:
    struct Line {
        double x1, y1, x2, y2;

		Line &operator*=(const double scale) {
			x1 *= scale;
			x2 *= scale;
			y1 *= scale;
			y2 *= scale;
			return *this;
		}

		void translate(const int x, const int y) {
			x1 += x;
			x2 += x;
			y1 += y;
			y2 += y;
		}
    };

    struct Face {
        std::size_t x, y, z;
    };

  public:
    Model3D(const std::string &filename);

    std::vector<Line> getSlice() const;

  private:
    std::vector<Vertex> vertices;
    std::vector<Face> faces;

    double layerHeight, currentLayer;

    inline double max_z(const Face &f) const {
        double z = vertices[f.x].z;
        z = std::max(z, vertices[f.y].z);
        z = std::max(z, vertices[f.z].z);
        return z;
    }

    inline double min_z(const Face &f) const {
        double z = vertices[f.x].z;
        z = std::min(z, vertices[f.y].z);
        z = std::min(z, vertices[f.z].z);
        return z;
    }

    inline std::vector<Vertex> getFace(const Face f) const {
        std::vector<Vertex> face;
        face.push_back(vertices[f.x]);
        face.push_back(vertices[f.y]);
        face.push_back(vertices[f.z]);
        return face;
    }

    inline Vertex getZPoint(const Vertex p1, const Vertex p2) const {
        Vertex v;
        v.x = p1.x + ((currentLayer - p1.z) * (p2.x - p1.x)) / (p2.z - p1.z);
        v.y = p1.y + ((currentLayer - p1.z) * (p2.y - p1.y)) / (p2.z - p1.z);
        v.z = currentLayer;
        return v;
    }
};

#endif
