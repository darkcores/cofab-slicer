#ifndef MODEL3D_H
#define MODEL3D_H

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include <QLine>
#include <QPoint>
#include <QPointF>
#include <QPolygon>
#include <QVector3D>

/**
 * Handle loading 3D models and calculating 2D intersections.
 */
class Model3D {
  public:
    Model3D(const std::string &filename);

	/**
	 * Generate all slices for this object.
	 */
    std::vector<std::vector<QPolygon>> getSlices();

	/**
	 * Get the XY-axes bounds of this objects.
	 */
	QRect getBounds() const {
		QPoint p1(x_left * INT_SCALE, y_left * INT_SCALE);
		QPoint p2(x_right * INT_SCALE, y_right * INT_SCALE);
		return QRect(p1, p2);
	}

	/**
	 * All points (floating point) are moved to positive values and
	 * scaled to integer representations. (for use with clipping algorithms)
	 */
    const long INT_SCALE = 1000000;

  private:
    std::vector<QVector3D> vertices;
    std::vector<std::array<std::size_t, 3>> faces;
    QPolygon previous_layer;
	float z_top = -99999, z_bottom = 99999;
	float x_left = 99999, x_right = -99999;
	float y_left = 99999, y_right = -99999;
    qreal layerHeight, currentLayer;

    std::vector<QLine> getLines() const;
    std::vector<QPolygon> getSlice() const;

    /**
     * Find max z value for triangle.
     */
    inline float max_z(const std::array<std::size_t, 3> &f) const {
        float z = vertices[f[0]].z();
        z = std::max(z, vertices[f[1]].z());
        z = std::max(z, vertices[f[2]].z());
        return z;
    }

    /**
     * Find min z value for triangle.
     */
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
