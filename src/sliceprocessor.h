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
    process(const std::vector<std::vector<QPolygon>> &paths);

  private:
    const QRect bounds;
    unsigned int num_walls = 2; // Min 1
    unsigned int num_rf = 2;    // Num roofs / floors
    const int infill_offset = 15;
	const int wall_width = 0.3771 * 1000; // extrusion width - layerheight * (1-pi/4)
    const long nozzle_offset = -wall_width;
    std::vector<ClipperLib::Paths> clipped_slices;

    /**
     * Clip QPolygons to paths (Xor).
     */
    ClipperLib::Paths clipPoly(const std::vector<QPolygon> &polys) const;
    std::vector<QPolygon> processSlice(const ClipperLib::Paths &clippedpaths,
                                       const std::size_t idx) const;
    ClipperLib::Paths getEdges(const ClipperLib::Paths &paths,
                               const bool first_path) const;
    void optimizeInfill(ClipperLib::Paths &infill) const;
    void optimizeEdges(ClipperLib::Paths &edges, std::size_t idx) const;
    /**
     * Get infill for region.
     */
    ClipperLib::Paths getInfill(const ClipperLib::Paths &edges) const;
    ClipperLib::Paths getDenseInfill(const ClipperLib::Paths &edges,
                                     const bool direction) const;

    /**
     * Get area that is covered, eroded and use that to compute the
     * difference with the infill area to get the dense infill area;
     */
    ClipperLib::Paths getDenseArea(const ClipperLib::Paths &infillArea,
                                   const std::size_t idx) const;
};

#endif
