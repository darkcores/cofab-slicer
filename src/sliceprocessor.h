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
	int num_walls = 2; // Min 1
	int infill_offset = 10;
    const long nozzle_offset = -0.4 * 10000;

    std::vector<QPolygon>
    processSlice(const std::vector<QPolygon> &paths) const;
    ClipperLib::Paths getEdges(const ClipperLib::Paths &paths,
                               const bool first_path) const;
	void optimizeInfill(ClipperLib::Paths &infill) const;
	/**
	 * Get infill for region.
	 */
    ClipperLib::Paths getInfill(const ClipperLib::Paths &edges) const;
	ClipperLib::Paths getDenseInfill(const ClipperLib::Paths & edges) const;
};

#endif
