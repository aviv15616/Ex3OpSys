// Utils.cpp - shared component
#include "../include/Utils.hpp"
#include <iostream>
#include <limits>

namespace common {

std::vector<Point> read_points(std::istream& in, int count) {
    std::vector<Point> points;
    for (int i = 0; i < count; ++i) {
        double x, y;
        if (in >> x >> y) {
            points.emplace_back(x, y);
        } else {
            throw std::runtime_error("שגיאה בקריאת נקודה");
        }
    }
    return points;
}

void skip_to_next_line(std::istream& in) {
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} // namespace common
