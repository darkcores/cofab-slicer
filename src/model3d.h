#ifndef MODEL3D_H
#define MODEL3D_H

#include <algorithm>
#include <string>
#include <array>
#include <vector>

#include <QPointF>
#include <QPoint>
#include <QLine>
#include <QPolygon>
#include <QVector3D>

/**
 * Handle loading 3D models and calculating 2D intersections.
 */
class Model3D {
  public:
    Model3D(const std::string &filename);

    std::vector<QPolygon> getSlice() const;
	const int INT_SCALE = 1000000;

  private:
    std::vector<QVector3D> vertices;
    std::vector<std::array<std::size_t, 3>> faces;
	std::vector<QLine> getLines() const;
	QPolygon previous_layer;

    qreal layerHeight, currentLayer;

    inline float max_z(const std::array<std::size_t, 3> &f) const {
        float z = vertices[f[0]].z();
        z = std::max(z, vertices[f[1]].z());
        z = std::max(z, vertices[f[2]].z());
        return z;
    }

    inline float min_z(const std::array<std::size_t, 3> &f) const {
        float z = vertices[f[0]].z();
        z = std::min(z, vertices[f[1]].z());
        z = std::min(z, vertices[f[2]].z());
        return z;
    }

    /**
     * Calculate point on current Z.
     * @param p1 first point index.
     * @param p2 second point index.
     */
    inline QPoint getZPoint(const std::size_t p1, const std::size_t p2) const {
        QPointF v;
        v.setX(vertices[p1].x() + ((currentLayer - vertices[p1].z()) *
                                   (vertices[p2].x() - vertices[p1].x())) /
                                      (vertices[p2].z() - vertices[p1].z()));
        v.setY(vertices[p1].y() + ((currentLayer - vertices[p1].z()) *
                                   (vertices[p2].y() - vertices[p1].y())) /
                                      (vertices[p2].z() - vertices[p1].z()));
		v *= INT_SCALE;
		return v.toPoint();
    }
};

#endif
