#ifndef SLICEPROCESSOR_H
#define SLICEPROCESSOR_H

#include <vector>
#include <QPolygonF>

class SliceProcessor {
 public:
	SliceProcessor();
	std::vector<QPolygonF> process(const std::vector<QPolygonF> &paths);
};

#endif
