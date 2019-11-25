#include "sliceprocessor.h"

#include <polyclipping/clipper.hpp>

SliceProcessor::SliceProcessor() {}

std::vector<QPolygonF> SliceProcessor::process(const std::vector<QPolygonF> &paths) {
	std::vector<QPolygonF> eroded;
	for (auto &path : paths) {
		for (auto &point : path) {
		}
	}
	return eroded;
}
