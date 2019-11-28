#ifndef SLICEPROCESSOR_H
#define SLICEPROCESSOR_H

#include <QPolygonF>
#include <vector>

#include <polyclipping/clipper.hpp>

class SliceProcessor {
  public:
    SliceProcessor();
    std::vector<QPolygon> process(const std::vector<QPolygon> &paths) const;

  private:
    const int nozzle_offset = -0.4 * 1000000;
    ClipperLib::Paths getEdges(const ClipperLib::Paths &paths,
                               const bool first_path) const;
    ClipperLib::Paths getInfill(const ClipperLib::Paths &edges,
                                const int direction = 0) const;
};

#endif
