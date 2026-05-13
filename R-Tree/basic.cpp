#ifndef BASIC_CPP
#define BASIC_CPP

#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <fstream>

using namespace std;

string trimValue(string s) {
    while (!s.empty() && isspace(s.front())) {
        s.erase(s.begin());
    }
    while (!s.empty() && isspace(s.back())) {
        s.pop_back();
    }
    return s;
}

// ===================== POINT CLASS =====================
class Point {
private:
    float x, y;
    string name;
    string category;

public:
    Point() : x(0), y(0), name(""), category("") {}

    Point(float x, float y, string name, string category) {
        this->x = x;
        this->y = y;
        this->name = trimValue(name);
        this->category = trimValue(category);
    }

    float getX() const { return x; }
    float getY() const { return y; }
    string getName() const { return name; }
    string getCategory() const { return category; }

    void save(ofstream& out) const {
        out << x << '\n';
        out << y << '\n';
        out << name << '\n';
        out << category << '\n';
    }

    static Point load(ifstream& in) {
        float x, y;
        string name, category;

        in >> x;
        in >> y;
        in.ignore();
        getline(in, name);
        getline(in, category);

        return Point(x, y, trimValue(name), trimValue(category));
    }
};

// ===================== RECTANGLE CLASS =====================
class Rectangle {
private:
    float minX, minY, maxX, maxY;

public:
    Rectangle() : minX(0), minY(0), maxX(0), maxY(0) {}

    Rectangle(float x, float y) : minX(x), minY(y), maxX(x), maxY(y) {}

    Rectangle(float minX, float minY, float maxX, float maxY) {
        this->minX = minX;
        this->minY = minY;
        this->maxX = maxX;
        this->maxY = maxY;
    }

    float getMinX() const { return minX; }
    float getMinY() const { return minY; }
    float getMaxX() const { return maxX; }
    float getMaxY() const { return maxY; }

    float area() const {
        return (maxX - minX) * (maxY - minY);
    }

    void expand(const Point& p) {
        minX = min(minX, p.getX());
        minY = min(minY, p.getY());
        maxX = max(maxX, p.getX());
        maxY = max(maxY, p.getY());
    }

    void expand(const Rectangle& r) {
        minX = min(minX, r.minX);
        minY = min(minY, r.minY);
        maxX = max(maxX, r.maxX);
        maxY = max(maxY, r.maxY);
    }

    bool contains(const Point& p) const {
        return (p.getX() >= minX && p.getX() <= maxX &&
                p.getY() >= minY && p.getY() <= maxY);
    }

    bool overlaps(const Rectangle& r) const {
        return !(maxX < r.minX || minX > r.maxX ||
                 maxY < r.minY || minY > r.maxY);
    }
};

// ===================== NODE CLASS =====================
class Node {
public:
    bool isLeaf;
    vector<Rectangle> rects;
    vector<Node*> children;
    vector<Point> points;

    Node(bool leaf) : isLeaf(leaf) {}
};
struct Query {
    string name = "";
    vector<string> categories;

    bool useRange = false;
    Rectangle range;

    bool useNearest = false;
    float x = 0, y = 0;

    bool useKNearest = false;
    int k = 1;
};

#endif // BASIC_CPP
