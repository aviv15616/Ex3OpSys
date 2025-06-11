// step3_interactive - main entry point
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include "../common/include/Point.hpp"
#include "../common/include/Graph.hpp"
#include "../common/include/CH.hpp"

using namespace std;
using namespace common;

void print_usage()
{
    cout << "Usage:\n"
         << "  Newgraph <n>           - Reads n points from next lines (format: x,y)\n"
         << "  Newpoint <x>,<y>       - Adds a point to the graph\n"
         << "  Removepoint <x>,<y>    - Removes a point from the graph\n"
         << "  CH                     - Computes and prints the convex hull\n"
         << endl;
}

int main()
{
    Graph graph;
    string line;

    print_usage(); // show usage instructions at start

    while (getline(cin, line))
    {
        stringstream ss(line);
        string command;
        ss >> command;

        if (command == "Newgraph")
        {
            int n;
            if (!(ss >> n))
            {
                cerr << "Error: Invalid format. Usage: Newgraph <n>" << endl;
                continue;
            }
            cout << "Enter " << n << " point(s) in format x,y" << endl;
            graph.clear();
            for (int i = 0; i < n; ++i)
            {
                string pointLine;
                if (!getline(cin, pointLine))
                {
                    cerr << "Error: Not enough points provided for Newgraph." << endl;
                    break;
                }

                replace(pointLine.begin(), pointLine.end(), ',', ' ');
                double x, y;
                stringstream ps(pointLine);
                if (ps >> x >> y)
                {
                    graph.add_point(Point(x, y));
                }
                else
                {
                    cerr << "Error: Invalid point format. Expected: x,y" << endl;
                }
            }
            cout << "Graph initialized with " << n << " points." << endl;
            cout << "You can now use:\n"
                 << "  Newpoint <x>,<y>       - Adds a point to the graph\n"
                 << "  Removepoint <x>,<y>    - Removes a point from the graph\n"
                 << "  CH                     - Computes and prints the convex hull\n"
                 << endl;
        }
        else if (command == "Newpoint")
        {
            double x, y;
            char comma;
            if (!(ss >> x >> comma >> y) || comma != ',')
            {
                cerr << "Error: Invalid format. Usage: Newpoint <x>,<y>" << endl;
                continue;
            }
            graph.add_point(Point(x, y));
        }
        else if (command == "Removepoint")
        {
            double x, y;
            char comma;
            if (!(ss >> x >> comma >> y) || comma != ',')
            {
                cerr << "Error: Invalid format. Usage: Removepoint <x>,<y>" << endl;
                continue;
            }
            graph.remove_point(Point(x, y));
        }
        else if (command == "CH")
        {
            auto hull = compute_convex_hull(graph.get_points());
            for (const auto &p : hull)
            {
                cout << p.x << "," << p.y << endl;
            }
            auto area = compute_area(hull);
            cout << "Area of convex hull: " << area << endl;
        }
        else
        {
            cerr << "Error: Unknown command: " << command << endl;
            print_usage();
        }
    }

    return 0;
}

