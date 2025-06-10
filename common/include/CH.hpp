#pragma once

#include "Point.hpp"
#include <vector>
#include <list>
#include <deque>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <type_traits>

namespace common {

    template <typename Container>
    Container compute_convex_hull(Container inputPoints) {
        using T = typename Container::value_type;

        if (inputPoints.size() < 3)
            return inputPoints;

        // מיון לפי סוג הקונטיינר
        if constexpr (std::is_same<Container, std::list<T>>::value) {
            inputPoints.sort();
        } else {
            std::sort(inputPoints.begin(), inputPoints.end());
        }

        Container hull;

        // פונקציה פנימית לחישוב cross product
        auto cross = [](const T& a, const T& b, const T& c) {
            return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
        };

        // בניית החצי התחתון
        for (const T& p : inputPoints) {
            while (std::distance(hull.begin(), hull.end()) >= 2) {
                auto r = std::prev(hull.end());
                auto q = std::prev(r);
                if (cross(*q, *r, p) <= 0)
                    hull.erase(r);
                else
                    break;
            }
            hull.insert(hull.end(), p);
        }

        // שמירת גודל החצי התחתון
        auto lower_size = hull.size();

        // בניית החצי העליון
        for (auto it = inputPoints.rbegin(); it != inputPoints.rend(); ++it) {
            const T& p = *it;
            while (hull.size() > lower_size) {
                auto r = std::prev(hull.end());
                auto q = std::prev(r);
                if (cross(*q, *r, p) <= 0)
                    hull.erase(r);
                else
                    break;
            }
            hull.insert(hull.end(), p);
        }

        // הסרת הנקודה הכפולה בסוף
        if (hull.size() > 1)
            hull.erase(std::prev(hull.end()));

        return hull;
    }

    template <typename Container>
    double compute_area(const Container& polygon) {
        if (polygon.size() < 3) return 0.0;

        auto it1 = polygon.begin();
        auto it2 = std::next(it1);
        double area = 0.0;

        while (it2 != polygon.end()) {
            area += (it1->x * it2->y - it2->x * it1->y);
            ++it1;
            ++it2;
        }

        // סגירת המעגל
        area += (polygon.rbegin()->x * polygon.begin()->y - polygon.begin()->x * polygon.rbegin()->y);
        return std::abs(area) / 2.0;
    }

}
