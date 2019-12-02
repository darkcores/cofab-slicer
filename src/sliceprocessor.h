#ifndef SLICEPROCESSOR_H
#define SLICEPROCESSOR_H

#include <QPolygonF>
#include <vector>

#include "polyclipping/clipper.hpp"

/**
 * Processing slices to paths for a 3D printer.
 */
class SliceProcessor {
  public:
    /**
     * Create a slice processor.  i
     * @param bounds Bounding rectangle used for clipping lines.
     */
    SliceProcessor(const QRect bounds);

	/**
	 * Process slices.
	 * @param paths slices.
	 */
    std::vector<std::vector<QPolygon>>
    process(const std::vector<std::vector<QPolygon>> &paths) const;

  private:
    const QRect bounds;
    const long nozzle_offset = -0.4 * 1000000;

    std::vector<QPolygon>
    processSlice(const std::vector<QPolygon> &paths) const;
    ClipperLib::Paths getEdges(const ClipperLib::Paths &paths,
                               const bool first_path) const;
    ClipperLib::Paths getInfill(const ClipperLib::Paths &edges) const;
};

#endif
