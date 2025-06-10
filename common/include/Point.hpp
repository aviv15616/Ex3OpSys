// Point.hpp - shared component
#ifndef POINT_HPP
#define POINT_HPP

namespace common {

struct Point {
    double x, y;

    Point(double x = 0, double y = 0) : x(x), y(y) {}

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    bool operator<(const Point& other) const {
        return (x < other.x) || (x == other.x && y < other.y);
    }
};

} // namespace common

#endif // POINT_HPP
