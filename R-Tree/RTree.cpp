#ifndef RTREE_CPP
#define RTREE_CPP

#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <fstream>

#include "basic.cpp"

const int MAX_ENTRIES = 3;
const int MIN_ENTRIES = (MAX_ENTRIES + 1) / 2;

using namespace std;


// ╔══════════════════════════════════════════════════════════════╗
//   RTree — spatial index using minimum bounding rectangles
// ╚══════════════════════════════════════════════════════════════╝
class RTree {
private:
    Node* root;


    // ════════════════════════════════════════════════════════════
    //  STRING UTILITIES
    // ════════════════════════════════════════════════════════════

    // Trims whitespace and lowercases a string for consistent comparisons
    string normalize(string s) {
        while (!s.empty() && isspace(s.front())) s.erase(s.begin());
        while (!s.empty() && isspace(s.back()))  s.pop_back();
        for (char& c : s) c = tolower(c);
        return s;
    }

    // ⚠ REDUNDANT — normalize() already lowercases; this is never called directly
    string toLowerCase(string s) {
        for (char& c : s) c = tolower(c);
        return s;
    }

    // Returns true if pointCategory matches any entry in the categories list (case-insensitive)
    bool categoryMatches(const string& pointCategory, const vector<string>& categories) {
        if (categories.empty()) return true;
        string pc = normalize(pointCategory);
        for (const auto& cat : categories)
            if (pc == normalize(cat)) return true;
        return false;
    }


    // ════════════════════════════════════════════════════════════
    //  TREE MEMORY MANAGEMENT
    // ════════════════════════════════════════════════════════════

    // Recursively frees all nodes in a subtree (post-order to avoid use-after-free)
    void deleteTree(Node* node) {
        if (node == nullptr) return;
        for (Node* child : node->children) deleteTree(child);
        delete node;
    }

    // Destroys the entire tree and replaces root with a fresh empty leaf
    void resetTree() {
        deleteTree(root);
        root = new Node(true);
    }


    // ════════════════════════════════════════════════════════════
    //  MBR (MINIMUM BOUNDING RECTANGLE) HELPERS
    // ════════════════════════════════════════════════════════════

    // Computes the tightest bounding rectangle that wraps all points/children of a node
    Rectangle getMBR(Node* node) {
        if (node->isLeaf) {
            if (node->points.empty()) return Rectangle(0, 0, 0, 0);
            Rectangle r(node->points[0].getX(), node->points[0].getY());
            for (size_t i = 0; i < node->points.size(); i++) r.expand(node->points[i]);
            return r;
        } else {
            if (node->children.empty()) return Rectangle(0, 0, 0, 0);
            Rectangle r = getMBR(node->children[0]);
            for (int i = 1; i < (int)node->children.size(); i++) r.expand(getMBR(node->children[i]));
            return r;
        }
    }

    // Returns how much rectangle r would grow in area if expanded to contain point p
    float enlargement(const Rectangle& r, const Point& p) {
        Rectangle expanded = r;
        expanded.expand(p);
        return expanded.area() - r.area();
    }

    // Returns the shortest Euclidean distance from point (x,y) to the nearest edge of rectangle r
    float rectDistance(const Rectangle& r, float x, float y) {
        float dx = max({r.getMinX() - x, 0.0f, x - r.getMaxX()});
        float dy = max({r.getMinY() - y, 0.0f, y - r.getMaxY()});
        return sqrt(dx * dx + dy * dy);
    }


    // ════════════════════════════════════════════════════════════
    //  INSERTION
    // ════════════════════════════════════════════════════════════

    // Moves the second half of an overflowing node's entries into a new sibling node
    Node* splitNode(Node* node) {
        Node* newNode = new Node(node->isLeaf);
        if (node->isLeaf) {
            int half = node->points.size() / 2;
            for (int i = half; i < (int)node->points.size(); i++) newNode->points.push_back(node->points[i]);
            node->points.erase(node->points.begin() + half, node->points.end());
        } else {
            int half = node->children.size() / 2;
            for (int i = half; i < (int)node->children.size(); i++) {
                newNode->children.push_back(node->children[i]);
                newNode->rects.push_back(node->rects[i]);
            }
            node->children.erase(node->children.begin() + half, node->children.end());
            node->rects.erase(node->rects.begin() + half, node->rects.end());
        }
        return newNode;
    }

    // Descends to the best leaf using least-enlargement, inserts point, splits on overflow
    Node* insertRecursive(Node* node, const Point& p) {
        if (node->isLeaf) {
            node->points.push_back(p);
            if (node->points.size() > MAX_ENTRIES) return splitNode(node);
            return nullptr;
        } else {
            float minInc = numeric_limits<float>::max();
            int best = 0;
            for (int i = 0; i < (int)node->rects.size(); i++) {
                float inc = enlargement(node->rects[i], p);
                if (inc < minInc) { minInc = inc; best = i; }
            }
            Node* splitChild = insertRecursive(node->children[best], p);
            node->rects[best] = getMBR(node->children[best]);
            if (splitChild) {
                node->children.push_back(splitChild);
                node->rects.push_back(getMBR(splitChild));
                if (node->children.size() > MAX_ENTRIES) return splitNode(node);
            }
            return nullptr;
        }
    }


    // ════════════════════════════════════════════════════════════
    //  DELETION
    // ════════════════════════════════════════════════════════════

    // Returns true if all four fields of two points match exactly
    bool samePoint(const Point& a, const Point& b) {
        return a.getX() == b.getX() &&
               a.getY() == b.getY() &&
               a.getName() == b.getName() &&
               a.getCategory() == b.getCategory();
    }

    // Returns true if a non-root node has fallen below the minimum entry count
    bool isUnderfull(Node* node) {
        if (node == root) return false;
        if (node->isLeaf) return (int)node->points.size() < MIN_ENTRIES;
        return (int)node->children.size() < MIN_ENTRIES;
    }

    // Collects all points whose normalised name exactly matches target into matches
    void collectPointsByNameRecursive(Node* node, const string& target, vector<Point>& matches) {
        if (node->isLeaf) {
            for (const auto& p : node->points)
                if (normalize(p.getName()) == target) matches.push_back(p);
            return;
        }
        for (Node* child : node->children)
            collectPointsByNameRecursive(child, target, matches);
    }

    // Removes a single point from the subtree; collects underfull node entries for reinsertion
    bool deleteRecursive(Node* node, const Point& target, vector<Point>& reinsertEntries) {
        if (node->isLeaf) {
            for (size_t i = 0; i < node->points.size(); i++) {
                if (samePoint(node->points[i], target)) {
                    node->points.erase(node->points.begin() + i);
                    return true;
                }
            }
            return false;
        }
        for (size_t i = 0; i < node->children.size(); i++) {
            if (!node->rects[i].contains(target)) continue;
            Node* child = node->children[i];
            if (!deleteRecursive(child, target, reinsertEntries)) continue;
            if (isUnderfull(child)) {
                collectAllPoints(child, reinsertEntries);
                deleteTree(child);
                node->children.erase(node->children.begin() + i);
                node->rects.erase(node->rects.begin() + i);
            } else {
                node->rects[i] = getMBR(child);
            }
            return true;
        }
        return false;
    }

    // Collapses a root with a single child, or replaces an empty internal root with a fresh leaf
    void adjustRootAfterDeletion() {
        while (!root->isLeaf && root->children.size() == 1) {
            Node* oldRoot = root;
            root = root->children[0];
            oldRoot->children.clear();
            delete oldRoot;
        }
        if (!root->isLeaf && root->children.empty()) {
            delete root;
            root = new Node(true);
        }
    }


    // ════════════════════════════════════════════════════════════
    //  RANGE & CATEGORY SEARCH
    // ════════════════════════════════════════════════════════════

    // Collects all points inside a rectangle, pruning subtrees whose MBR doesn't overlap
    void searchRecursive(Node* node, const Rectangle& range, vector<Point>& res) {
        if (node->isLeaf) {
            for (size_t i = 0; i < node->points.size(); i++)
                if (range.contains(node->points[i])) res.push_back(node->points[i]);
        } else {
            for (int i = 0; i < (int)node->rects.size(); i++)
                if (node->rects[i].overlaps(range)) searchRecursive(node->children[i], range, res);
        }
    }

    // Full tree scan collecting points matching any of the given categories (no spatial pruning)
    void searchByCategoryRecursive(Node* node, const vector<string>& categories, vector<Point>& result) {
        if (node->isLeaf) {
            for (size_t i = 0; i < node->points.size(); i++)
                if (categoryMatches(node->points[i].getCategory(), categories)) result.push_back(node->points[i]);
        } else {
            for (size_t i = 0; i < node->children.size(); i++)
                searchByCategoryRecursive(node->children[i], categories, result);
        }
    }

    // Range search with an additional category filter; prunes non-overlapping subtrees
    void searchByCategoryInRangeRecursive(Node* node, const Rectangle& range, const vector<string>& categories, vector<Point>& result) {
        if (node->isLeaf) {
            for (size_t i = 0; i < node->points.size(); i++)
                if (range.contains(node->points[i]) && categoryMatches(node->points[i].getCategory(), categories))
                    result.push_back(node->points[i]);
        } else {
            for (int i = 0; i < (int)node->rects.size(); i++)
                if (node->rects[i].overlaps(range))
                    searchByCategoryInRangeRecursive(node->children[i], range, categories, result);
        }
    }

    // Full tree scan collecting every point regardless of category or location
    void collectAllPoints(Node* node, vector<Point>& result) {
        if (node->isLeaf) {
            for (size_t i = 0; i < node->points.size(); i++) result.push_back(node->points[i]);
        } else {
            for (size_t i = 0; i < node->children.size(); i++) collectAllPoints(node->children[i], result);
        }
    }


    // ════════════════════════════════════════════════════════════
    //  NAME SEARCH
    // ════════════════════════════════════════════════════════════

    // Full tree scan; returns points whose normalised name contains the target substring
    void searchByNameRecursive(Node* node, const string& targetName, vector<Point>& result) {
        string target = normalize(targetName);
        if (node->isLeaf) {
            for (size_t i = 0; i < node->points.size(); i++) {
                string currentName = normalize(node->points[i].getName());
                if (currentName.find(target) != string::npos) result.push_back(node->points[i]);
            }
            return;
        }
        for (size_t i = 0; i < node->children.size(); i++)
            searchByNameRecursive(node->children[i], targetName, result);
    }

    // Depth-first exact-match name search; stops and returns true on first hit
    bool findByName(Node* node, const string& targetName, Point& found) {
        if (node->isLeaf) {
            for (const auto& p : node->points)
                if (p.getName() == targetName) { found = p; return true; }
            return false;
        }
        for (Node* child : node->children)
            if (findByName(child, targetName, found)) return true;
        return false;
    }


    // ════════════════════════════════════════════════════════════
    //  NEAREST NEIGHBOR
    // ════════════════════════════════════════════════════════════

    // Branch-and-bound NN search; visits children in MBR-distance order and prunes far subtrees
    void nearestRecursive(Node* node, float x, float y, Point& best, float& bestDist) {
        if (node->isLeaf) {
            for (const auto& p : node->points) {
                float d = sqrt(pow(p.getX() - x, 2) + pow(p.getY() - y, 2));
                if (d < bestDist) { bestDist = d; best = p; }
            }
        } else {
            vector<pair<float, int>> order;
            for (int i = 0; i < (int)node->rects.size(); i++)
                order.push_back({rectDistance(node->rects[i], x, y), i});
            sort(order.begin(), order.end());
            for (auto& [dist, idx] : order)
                if (dist < bestDist) nearestRecursive(node->children[idx], x, y, best, bestDist);
        }
    }

    // Same as nearestRecursive but skips any point whose name equals skipName (avoids self-match)
    void nearestRecursiveExclude(Node* node, float x, float y, const string& skipName, Point& best, float& bestDist) {
        if (node->isLeaf) {
            for (const auto& p : node->points) {
                if (p.getName() == skipName) continue;
                float d = sqrt(pow(p.getX() - x, 2) + pow(p.getY() - y, 2));
                if (d < bestDist) { bestDist = d; best = p; }
            }
        } else {
            vector<pair<float, int>> order;
            for (int i = 0; i < (int)node->rects.size(); i++)
                order.push_back({rectDistance(node->rects[i], x, y), i});
            sort(order.begin(), order.end());
            for (auto& [dist, idx] : order)
                if (dist < bestDist) nearestRecursiveExclude(node->children[idx], x, y, skipName, best, bestDist);
        }
    }

    // Branch-and-bound NN search restricted to points matching one of the given categories
    void nearestRecursiveInCategories(Node* node, float x, float y, const vector<string>& categories, Point& best, float& bestDist) {
        if (node->isLeaf) {
            for (const auto& p : node->points) {
                if (!categoryMatches(p.getCategory(), categories)) continue;
                float d = sqrt(pow(p.getX() - x, 2) + pow(p.getY() - y, 2));
                if (d < bestDist) { bestDist = d; best = p; }
            }
        } else {
            vector<pair<float, int>> order;
            for (int i = 0; i < (int)node->rects.size(); i++)
                order.push_back({rectDistance(node->rects[i], x, y), i});
            sort(order.begin(), order.end());
            for (auto& [dist, idx] : order)
                if (dist < bestDist) nearestRecursiveInCategories(node->children[idx], x, y, categories, best, bestDist);
        }
    }

    // Branch-and-bound NN search restricted to a single category string (used by bestPlacementSuggestion)
    void nearestInCategoryRecursive(Node* node, float qx, float qy, const string& category, Point& best, float& bestDist) {
        if (node->isLeaf) {
            for (const auto& p : node->points) {
                if (normalize(p.getCategory()) != normalize(category)) continue;
                float d = sqrt(pow(p.getX() - qx, 2) + pow(p.getY() - qy, 2));
                if (d < bestDist) { bestDist = d; best = p; }
            }
        } else {
            vector<pair<float, int>> order;
            for (int i = 0; i < (int)node->rects.size(); i++)
                order.push_back({rectDistance(node->rects[i], qx, qy), i});
            sort(order.begin(), order.end());
            for (auto& [dist, idx] : order)
                if (dist < bestDist) nearestInCategoryRecursive(node->children[idx], qx, qy, category, best, bestDist);
        }
    }


    // ════════════════════════════════════════════════════════════
    //  K-NEAREST NEIGHBOR
    // ════════════════════════════════════════════════════════════

    // Branch-and-bound kNN; maintains a sorted top-k list and prunes subtrees that can't improve it
    void k_nearestRecursive(Node* node, float x, float y, int k, vector<pair<float, Point>>& bestList) {
        if (node->isLeaf) {
            for (const auto& p : node->points) {
                float d = sqrt(pow(p.getX() - x, 2) + pow(p.getY() - y, 2));
                bestList.push_back({d, p});
                sort(bestList.begin(), bestList.end(), [](const pair<float, Point>& a, const pair<float, Point>& b) { return a.first < b.first; });
                if (bestList.size() > (size_t)k) bestList.pop_back();
            }
        } else {
            vector<pair<float, int>> order;
            for (int i = 0; i < (int)node->rects.size(); i++)
                order.push_back({rectDistance(node->rects[i], x, y), i});
            sort(order.begin(), order.end());
            for (auto& [dist, idx] : order)
                if (bestList.size() < (size_t)k || dist < bestList.back().first)
                    k_nearestRecursive(node->children[idx], x, y, k, bestList);
        }
    }

    // Same as k_nearestRecursive but skips points not matching the given categories
    void k_nearestRecursiveInCategories(Node* node, float x, float y, int k, const vector<string>& categories, vector<pair<float, Point>>& bestList) {
        if (node->isLeaf) {
            for (const auto& p : node->points) {
                if (!categoryMatches(p.getCategory(), categories)) continue;
                float d = sqrt(pow(p.getX() - x, 2) + pow(p.getY() - y, 2));
                bestList.push_back({d, p});
                sort(bestList.begin(), bestList.end(), [](const pair<float, Point>& a, const pair<float, Point>& b) { return a.first < b.first; });
                if (bestList.size() > (size_t)k) bestList.pop_back();
            }
        } else {
            vector<pair<float, int>> order;
            for (int i = 0; i < (int)node->rects.size(); i++)
                order.push_back({rectDistance(node->rects[i], x, y), i});
            sort(order.begin(), order.end());
            for (auto& [dist, idx] : order)
                if (bestList.size() < (size_t)k || dist < bestList.back().first)
                    k_nearestRecursiveInCategories(node->children[idx], x, y, k, categories, bestList);
        }
    }


    // ════════════════════════════════════════════════════════════
    //  FILE I/O HELPERS
    // ════════════════════════════════════════════════════════════

    // Recursively writes a node and all its descendants to an output file stream
    void saveNode(ofstream& out, Node* node) {
        out << node->isLeaf << '\n';
        if (node->isLeaf) {
            out << node->points.size() << '\n';
            for (const auto& p : node->points) p.save(out);
        } else {
            out << node->children.size() << '\n';
            for (Node* child : node->children) saveNode(out, child);
        }
    }

    // Recursively reconstructs a node and all its descendants from a file stream
    Node* loadNode(ifstream& in) {
        bool isLeaf; in >> isLeaf;
        int count;   in >> count;
        Node* node = new Node(isLeaf);
        if (isLeaf) {
            for (int i = 0; i < count; i++) node->points.push_back(Point::load(in));
        } else {
            for (int i = 0; i < count; i++) {
                Node* child = loadNode(in);
                node->children.push_back(child);
                node->rects.push_back(getMBR(child));
            }
        }
        return node;
    }


    // ════════════════════════════════════════════════════════════
    //  DEBUG HELPER
    // ════════════════════════════════════════════════════════════

    // Prints the full tree structure to stdout with indentation showing depth
    void printRecursive(Node* node, int depth) {
        string indent(depth * 4, ' ');
        if (node->isLeaf) {
            cout << indent << "[Leaf Node] Points: ";
            for (const auto& p : node->points)
                cout << p.getName() << "(" << p.getX() << "," << p.getY() << ") ";
            cout << endl;
        } else {
            cout << indent << "[Internal Node] MBRs count: " << node->rects.size() << endl;
            for (int i = 0; i < (int)node->children.size(); i++) {
                Rectangle r = node->rects[i];
                cout << indent << "  Child " << i << " MBR: ["
                     << r.getMinX() << "," << r.getMinY() << "] to ["
                     << r.getMaxX() << "," << r.getMaxY() << "]\n";
                printRecursive(node->children[i], depth + 1);
            }
        }
    }


public:

    // ════════════════════════════════════════════════════════════
    //  CONSTRUCTOR / DESTRUCTOR
    // ════════════════════════════════════════════════════════════

    // Initialises the tree with a single empty leaf as the root
    RTree() { root = new Node(true); }

    // Frees the entire tree on destruction
    ~RTree() { deleteTree(root); }


    // ════════════════════════════════════════════════════════════
    //  INSERT / DELETE / UPDATE
    // ════════════════════════════════════════════════════════════

    // Public entry point for insertion; handles root-split and new-root creation
    void insert(const Point& p) {
        Node* splitChild = insertRecursive(root, p);
        if (splitChild) {
            Node* newRoot = new Node(false);
            newRoot->children.push_back(root);
            newRoot->rects.push_back(getMBR(root));
            newRoot->children.push_back(splitChild);
            newRoot->rects.push_back(getMBR(splitChild));
            root = newRoot;
        }
    }

    // Finds all exact-name matches, removes each via deleteRecursive, reinserts displaced entries
    int deleteByName(const string& placeName) {
        string target = normalize(placeName);
        vector<Point> matches;
        collectPointsByNameRecursive(root, target, matches);
        int deletedCount = 0;
        for (const auto& match : matches) {
            vector<Point> reinsertEntries;
            if (!deleteRecursive(root, match, reinsertEntries)) continue;
            deletedCount++;
            adjustRootAfterDeletion();
            for (const auto& p : reinsertEntries) insert(p);
            adjustRootAfterDeletion();
        }
        return deletedCount;
    }

    bool deletePoint(const Point& target) {
        vector<Point> reinsertEntries;
        if (!deleteRecursive(root, target, reinsertEntries)) return false;
        adjustRootAfterDeletion();
        for (const auto& p : reinsertEntries) insert(p);
        adjustRootAfterDeletion();
        return true;
    }

    bool updatePoint(const Point& target, const Point& updatedPoint) {
        if (!deletePoint(target)) return false;
        insert(updatedPoint);
        return true;
    }

    // Replaces the first point whose normalised name matches oldName with updatedPoint; rebuilds the tree
    bool updateByName(const string& oldName, const Point& updatedPoint) {
        vector<Point> allPoints = getAllPoints();
        string target = normalize(oldName);
        bool updated = false;
        for (auto& p : allPoints) {
            if (!updated && normalize(p.getName()) == target) { p = updatedPoint; updated = true; }
        }
        if (!updated) return false;
        resetTree();
        for (const auto& p : allPoints) insert(p);
        return true;
    }


    // ════════════════════════════════════════════════════════════
    //  RANGE & CATEGORY SEARCH
    // ════════════════════════════════════════════════════════════

    // Returns all points whose coordinates fall inside the given rectangle
    vector<Point> search(const Rectangle& range) {
        vector<Point> res;
        searchRecursive(root, range, res);
        return res;
    }

    // Returns every point in the tree (no filter)
    vector<Point> getAllPoints() {
        vector<Point> result;
        collectAllPoints(root, result);
        return result;
    }

    // Returns all points matching any of the given categories (full tree scan)
    vector<Point> searchByCategory(const vector<string>& categories) {
        vector<Point> result;
        searchByCategoryRecursive(root, categories, result);
        return result;
    }

    // Returns points matching the given categories that also fall inside the rectangle
    vector<Point> searchByCategoryInRange(const Rectangle& range, const vector<string>& categories) {
        vector<Point> result;
        searchByCategoryInRangeRecursive(root, range, categories, result);
        return result;
    }

    // ⚠ REDUNDANT — executeQuery already applies filterByCategory after building results;
    //   calling this on an already-fetched list duplicates that work; prefer searchByCategory()
    vector<Point> filterByCategory(const vector<Point>& points, const vector<string>& categories) {
        vector<Point> result;
        for (const auto& p : points)
            if (categoryMatches(p.getCategory(), categories)) result.push_back(p);
        return result;
    }


    // ════════════════════════════════════════════════════════════
    //  NAME SEARCH
    // ════════════════════════════════════════════════════════════

    // Returns all points whose name contains the query string (case-insensitive partial match)
    vector<Point> searchByName(const string& name) {
        vector<Point> result;
        searchByNameRecursive(root, name, result);
        return result;
    }

    // Finds the first point with an exact name match; returns true and fills found on success
    bool findPlaceByName(const string& placeName, Point& found) {
        return findByName(root, placeName, found);
    }


    // ════════════════════════════════════════════════════════════
    //  NEAREST NEIGHBOR
    // ════════════════════════════════════════════════════════════

    // Returns the single closest point in the tree to (x, y)
    Point nearestNeighbor(float x, float y) {
        Point best;
        float bestDist = numeric_limits<float>::max();
        nearestRecursive(root, x, y, best, bestDist);
        return best;
    }

    // Returns the nearest point to (x, y) restricted to the given categories; false if none found
    bool nearestNeighborInCategories(float x, float y, const vector<string>& categories, Point& best) {
        float bestDist = numeric_limits<float>::max();
        nearestRecursiveInCategories(root, x, y, categories, best, bestDist);
        return bestDist != numeric_limits<float>::max();
    }

    // ⚠ REDUNDANT — nearestNeighborByPlaceName does the same correctly and excludes the source;
    //   this version can incorrectly return the source place as its own nearest neighbor
    Point nearestNeighborByName(const string& placeName) {
        Point source;
        bool exists = findPlaceByName(placeName, source);
        if (!exists) { cout << "Place not found.\n"; return Point(); }
        Point best;
        float bestDist = numeric_limits<float>::max();
        nearestRecursive(root, source.getX(), source.getY(), best, bestDist);
        return best;
    }

    // Finds the nearest point to a named place, excluding the place itself from results
    Point nearestNeighborByPlaceName(const string& placeName) {
        Point source;
        bool exists = findPlaceByName(placeName, source);
        if (!exists) { cout << "Place not found.\n"; return Point(); }
        Point best;
        float bestDist = numeric_limits<float>::max();
        nearestRecursiveExclude(root, source.getX(), source.getY(), source.getName(), best, bestDist);
        return best;
    }


    // ════════════════════════════════════════════════════════════
    //  K-NEAREST NEIGHBOR
    // ════════════════════════════════════════════════════════════

    // Returns the k closest points to (x, y) across all categories
    vector<Point> k_nearestNeighbors(float x, float y, int k) {
        vector<pair<float, Point>> bestList;
        k_nearestRecursive(root, x, y, k, bestList);
        vector<Point> result;
        for (auto& p : bestList) result.push_back(p.second);
        return result;
    }

    // Returns the k closest points to (x, y) restricted to the given categories
    vector<Point> k_nearestNeighborsInCategories(float x, float y, int k, const vector<string>& categories) {
        vector<pair<float, Point>> bestList;
        k_nearestRecursiveInCategories(root, x, y, k, categories, bestList);
        vector<Point> result;
        for (auto& p : bestList) result.push_back(p.second);
        return result;
    }


    // ════════════════════════════════════════════════════════════
    //  PLACEMENT SUGGESTION
    // ════════════════════════════════════════════════════════════

    // Samples a grid over the map and returns the spot farthest from any existing point of the given category
    Point bestPlacementSuggestion(const string& category, int gridSize = 20) {
        vector<string> catFilter = {category};
        vector<Point> categoryPoints = searchByCategory(catFilter);
        if (categoryPoints.empty()) {
            cout << "No existing points of category '" << category << "' found. Cannot compute suggestion.\n";
            return Point();
        }
        vector<Point> allPoints = getAllPoints();
        float minX = allPoints[0].getX(), maxX = allPoints[0].getX();
        float minY = allPoints[0].getY(), maxY = allPoints[0].getY();
        for (const auto& p : allPoints) {
            if (p.getX() < minX) minX = p.getX();
            if (p.getX() > maxX) maxX = p.getX();
            if (p.getY() < minY) minY = p.getY();
            if (p.getY() > maxY) maxY = p.getY();
        }
        float marginX = (maxX - minX) * 0.05f;
        float marginY = (maxY - minY) * 0.05f;
        minX -= marginX; maxX += marginX;
        minY -= marginY; maxY += marginY;
        float stepX = (maxX - minX) / gridSize;
        float stepY = (maxY - minY) / gridSize;
        float bestGap = -1.0f, suggestX = minX, suggestY = minY;
        for (int row = 0; row <= gridSize; row++) {
            for (int col = 0; col <= gridSize; col++) {
                float cx = minX + col * stepX;
                float cy = minY + row * stepY;
                Point nearestCat;
                float nearestDist = numeric_limits<float>::max();
                nearestInCategoryRecursive(root, cx, cy, category, nearestCat, nearestDist);
                if (nearestDist > bestGap) { bestGap = nearestDist; suggestX = cx; suggestY = cy; }
            }
        }
        cout << "Suggested location for new '" << category << "':\n";
        cout << "  Coordinates : (" << suggestX << ", " << suggestY << ")\n";
        cout << "  Gap distance: " << bestGap << " units from nearest existing " << category << "\n";
        return Point(suggestX, suggestY, "Suggested Location", category);
    }


    // ════════════════════════════════════════════════════════════
    //  QUERY DISPATCHER
    // ════════════════════════════════════════════════════════════

    // Routes a Query struct to the correct search method and applies optional category filtering
    vector<Point> executeQuery(const Query& q) {
        vector<Point> result;
        if (!q.name.empty()) {
            result = searchByName(q.name);
        } else if (q.useRange) {
            result = q.categories.empty() ? search(q.range) : searchByCategoryInRange(q.range, q.categories);
        } else if (q.useKNearest) {
            result = q.categories.empty() ? k_nearestNeighbors(q.x, q.y, q.k) : k_nearestNeighborsInCategories(q.x, q.y, q.k, q.categories);
        } else if (q.useNearest) {
            Point nearest;
            bool found = false;
            if (q.categories.empty()) { nearest = nearestNeighbor(q.x, q.y); found = true; }
            else found = nearestNeighborInCategories(q.x, q.y, q.categories, nearest);
            if (found) result.push_back(nearest);
        } else {
            result = q.categories.empty() ? getAllPoints() : searchByCategory(q.categories);
        }
        if (!q.categories.empty() && !q.useRange && !q.useKNearest && !q.useNearest)
            result = filterByCategory(result, q.categories);
        return result;
    }


    // ════════════════════════════════════════════════════════════
    //  FILE I/O
    // ════════════════════════════════════════════════════════════

    // Serialises the entire tree to a plain-text file, node by node
    void saveTreeToFile(const string& filename) {
        ofstream out(filename);
        if (!out.is_open()) { cout << "Could not open .dat file for saving.\n"; return; }
        saveNode(out, root);
        out.close();
    }

    // Destroys the current tree and reconstructs it from a previously saved file
    void loadTreeFromFile(const string& filename) {
        ifstream in(filename);
        if (!in.is_open()) { cout << "Could not open .dat file for loading.\n"; return; }
        deleteTree(root);
        root = loadNode(in);
        in.close();
    }

    // Returns true if a save file already exists at the given path
    bool treeFileExists(const string& filename) {
        ifstream f(filename);
        return f.good();
    }


    // ════════════════════════════════════════════════════════════
    //  DEBUG
    // ════════════════════════════════════════════════════════════

    // Prints the full tree structure with indented nodes and MBR coordinates to stdout
    void print() {
        cout << "--- R-Tree Structure ---\n";
        printRecursive(root, 0);
        cout << "------------------------\n";
    }
};

#endif // RTREE_CPP
