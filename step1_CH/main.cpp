#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <list>    // או deque/vector לפי הצורך
#include "Point.hpp"
#include "CH.hpp"

using namespace std;
using namespace common;

int main() {
    int num_points;
    cout << "Enter the number of points: ";
    cin >> num_points;
    cin.ignore(); // מדלג על שורת רווח שנשארת אחרי הקריאה ל־cin

    using Container = std::list<Point>; 
    Container points;

    cout << "Enter the points as x,y (each on a new line):\n";

    for (int i = 0; i < num_points; ++i) {
        string line;
        getline(cin, line);
        replace(line.begin(), line.end(), ',', ' ');

        double x, y;
        stringstream ss(line);
        if (ss >> x >> y) {
            points.emplace_back(x, y);
        } else {
            cerr << "Invalid input for point #" << (i + 1) << ": " << line << endl;
            --i;
        }
    }

    auto hull = compute_convex_hull(points);
    double area = compute_area(hull);

    cout << "Area of convex hull: " << area << endl;

    return 0;
}
