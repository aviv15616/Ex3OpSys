// step3_interactive - main entry point
#include <iostream>
#include <sstream>
#include <string>
#include "../common/include/Point.hpp"
#include "../common/include/Graph.hpp"
#include "../common/include/CH.hpp"

using namespace std;
using namespace common;

int main() {
    Graph graph;
    string line;

    while (getline(cin, line)) {
        stringstream ss(line);
        string command;
        ss >> command;

        if (command == "Newgraph") {
            int n;
            ss >> n;
            graph.clear();

            for (int i = 0; i < n; ++i) {
                string pointLine;
                getline(cin, pointLine);
                replace(pointLine.begin(), pointLine.end(), ',', ' ');

                double x, y;
                stringstream ps(pointLine);
                if (ps >> x >> y) {
                    graph.add_point(Point(x, y));
                }
            }

        } else if (command == "Newpoint") {
            double x, y;
            char comma;
            ss >> x >> comma >> y;
            graph.add_point(Point(x, y));

        } else if (command == "Removepoint") {
            double x, y;
            char comma;
            ss >> x >> comma >> y;
            graph.remove_point(Point(x, y));

        } else if (command == "CH") {
            auto hull = compute_convex_hull(graph.get_points());
            for (const auto& p : hull) {
                cout << p.x << " " << p.y << endl;
            }

        } else {
            cerr << "Unknown command: " << command << endl;
        }
    }

    return 0;
}
