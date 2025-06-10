#include <iostream>
#include <vector>
#include <cassert>
#include "CH.hpp"
#include "Point.hpp"
#include "Graph.hpp"

using namespace std;
using namespace common;

void test_triangle() {
    vector<Point> pts = {{0, 0}, {1, 0}, {0, 1}};
    auto hull = compute_convex_hull(pts);
    double area = compute_area(hull);
    assert(hull.size() == 3);
    assert(abs(area - 0.5) < 1e-6);
    cout << "test_triangle passed\n";
}

void test_rectangle() {
    vector<Point> pts = {{0, 0}, {2, 0}, {2, 1}, {0, 1}};
    auto hull = compute_convex_hull(pts);
    double area = compute_area(hull);
    assert(hull.size() == 4);
    assert(abs(area - 2.0) < 1e-6);
    cout << "test_rectangle passed\n";
}

void test_line_points() {
    vector<Point> pts = {{0, 0}, {1, 0}, {2, 0}, {3, 0}};
    auto hull = compute_convex_hull(pts);
    double area = compute_area(hull);
    assert(hull.size() == 2); // רק הקצוות נשארים
    assert(area == 0.0);
    cout << "test_line_points passed\n";
}

void test_single_point() {
    vector<Point> pts = {{5, 5}};
    auto hull = compute_convex_hull(pts);
    double area = compute_area(hull);
    assert(hull.size() == 1);
    assert(area == 0.0);
    cout << "test_single_point passed\n";
}

void test_duplicate_points() {
    vector<Point> pts = {{1, 1}, {2, 2}, {2, 2}, {3, 3}};
    auto hull = compute_convex_hull(pts);
    double area = compute_area(hull);
    assert(area == 0.0); // כולן על קו ישר
    assert(hull.size() == 2);
    cout << "test_duplicate_points passed\n";
}

int main() {
    test_triangle();
    test_rectangle();
    test_line_points();
    test_single_point();
    test_duplicate_points();

    cout << "✅ All tests passed!" << endl;
    return 0;
}
