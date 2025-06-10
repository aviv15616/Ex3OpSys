#include <iostream>
#include <list>
#include <random>
#include "Point.hpp"
#include "CH.hpp"

using namespace std;
using namespace common;

int main() {
    list<Point> points;

    // יצירת 100,000 נקודות רנדומליות
    mt19937 gen(42);
    uniform_real_distribution<> dis(-1000.0, 1000.0);
    for (int i = 0; i < 1000000; ++i) {
        points.emplace_back(dis(gen), dis(gen));
    }

    auto hull = compute_convex_hull(points);
    double area = compute_area(hull);
    cout << "Convex hull area (list): " << area << endl;
    cout << "Hull size: " << hull.size() << endl;

    return 0;
}
