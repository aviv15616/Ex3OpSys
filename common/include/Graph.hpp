// Graph.hpp - shared component
#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include "Point.hpp"

namespace common {

class Graph {
private:
    std::vector<Point> points;

public:
    void add_point(const Point& p);
    void remove_point(const Point& p);
    const std::vector<Point>& get_points() const;
    void clear();
};

} // namespace common

#endif // GRAPH_HPP
