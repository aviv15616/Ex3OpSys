// Utils.hpp - shared component
#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <istream>
#include "Point.hpp"

namespace common {

std::vector<Point> read_points(std::istream& in, int count);
void skip_to_next_line(std::istream& in);

} // namespace common

#endif // UTILS_HPP
