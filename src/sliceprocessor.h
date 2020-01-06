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

	void setWalls(unsigned int w) { num_walls = w; }
	void setRoofsFloors(unsigned int rf) { num_rf = rf; }
	void setInfillSpacing(int spacing) { infill_offset = spacing; }
	void setNozzleWidth(double w);
	void setLayerHeight(double h);
	void setSupport(bool enabled) { support = enabled; };

  private:
	bool support = false;
	double nozzleWidth = 0.4;
	double layerHeight = 0.2;
    const QRect bounds;
    unsigned int num_walls = 2; // Min 1
    unsigned int num_rf = 2;    // Num roofs / floors
    int infill_offset = 15;
	// Based on: https://manual.slic3r.org/advanced/flow-math
	int wall_width = 0.3771 * 1000; // extrusion width - layerheight * (1-pi/4)
    long nozzle_offset = -wall_width;
    std::vector<ClipperLib::Paths> clipped_slices;

    bool firstLayerOffSupport(const ClipperLib::Paths &i, const ClipperLib::Paths &j, const int offset) const;
    void addSupport(std::vector<std::vector<QPolygon>> &processed);
    ClipperLib::Paths getGrid(const int gridSpace, const ClipperLib::Paths diff);

    ClipperLib::Paths getOffsetEdges(const ClipperLib::Paths &paths, const int offset) const ;

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
    ClipperLib::Paths getInfillSupport(const ClipperLib::Paths &edges) const;

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
