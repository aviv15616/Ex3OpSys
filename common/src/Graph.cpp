// Graph.cpp - shared component
#include "../include/Graph.hpp"
#include <algorithm>

namespace common {

void Graph::add_point(const Point& p) {
    points.push_back(p);
}

void Graph::remove_point(const Point& p) {
    points.erase(std::remove(points.begin(), points.end(), p), points.end());
}

const std::vector<Point>& Graph::get_points() const {
    return points;
}

void Graph::clear() {
    points.clear();
}

} // namespace common
