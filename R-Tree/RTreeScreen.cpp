#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <limits.h>
#include <unistd.h>

#include "builder.cpp"
#include "region_ranking.cpp"
#include "AreaCoverage.cpp"

using namespace std;

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 760;
const float SIDEBAR_WIDTH = 330.0f;
const float TOP_BAR_HEIGHT = 74.0f;
const float PADDING = 18.0f;
const float POINT_DOT_RADIUS = 2.5f;
const float RESULT_RING_RADIUS = 10.0f;
const float RESULT_SELECT_RADIUS = 20.0f;
const float POINT_SELECT_RADIUS = 12.0f;

struct MapBounds {
    float minX = 0;
    float maxX = 1;
    float minY = 0;
    float maxY = 1;
};

struct Button {
    sf::FloatRect bounds;
    string label;
    bool selected = false;
};

struct TextBox {
    sf::FloatRect bounds;
    string label;
    string value;
    bool active = false;
};

int activeTextBoxIndex(const vector<TextBox>& inputs) {
    for (int i = 0; i < (int)inputs.size(); i++) {
        if (inputs[i].active) return i;
    }
    return -1;
}

bool isEditableTextBox(int index, const string& mode) {
    if (mode == "Update") return index == 0;
    if (index == 2 || index == 3) return mode == "Insert" || mode == "Delete";
    return true;
}

bool isVisibleTextBox(int index) {
    return true;
}

int nextEditableTextBoxIndex(int activeIndex, int inputCount, const string& mode) {
    for (int offset = 1; offset <= inputCount; offset++) {
        int nextIndex = activeIndex < 0 ? offset - 1 : (activeIndex + offset) % inputCount;
        if (isEditableTextBox(nextIndex, mode)) return nextIndex;
    }
    return -1;
}

string trimScreenText(string s) {
    while (!s.empty() && isspace((unsigned char)s.front())) s.erase(s.begin());
    while (!s.empty() && isspace((unsigned char)s.back())) s.pop_back();
    return s;
}

bool parseFloatStrict(const string& text, float& value) {
    string trimmed = trimScreenText(text);
    if (trimmed.empty()) return false;
    try {
        size_t read = 0;
        value = stof(trimmed, &read);
        return read == trimmed.size();
    } catch (...) {
        return false;
    }
}

float parseFloatOrDefault(const string& text, float fallback) {
    try {
        return stof(text);
    } catch (...) {
        return fallback;
    }
}

int parseIntOrDefault(const string& text, int fallback) {
    try {
        return max(1, stoi(text));
    } catch (...) {
        return fallback;
    }
}

string pointLine(const Point& p) {
    stringstream ss;
    ss << fixed << setprecision(4)
       << p.getName() << " [" << p.getCategory() << "]  "
       << "(" << p.getX() << ", " << p.getY() << ")";
    return ss.str();
}

bool sameScreenPoint(const Point& a, const Point& b) {
    return a.getName() == b.getName() &&
           a.getCategory() == b.getCategory() &&
           fabs(a.getX() - b.getX()) < 0.00001f &&
           fabs(a.getY() - b.getY()) < 0.00001f;
}

float pointDistance(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return sqrt(dx * dx + dy * dy);
}

string normalizeScreenText(string s) {
    s = trimScreenText(s);
    for (char& ch : s) ch = (char)tolower((unsigned char)ch);
    return s;
}

bool samePlaceIdentity(const Point& point, const string& name, const string& category) {
    return normalizeScreenText(point.getName()) == normalizeScreenText(name) &&
           normalizeScreenText(point.getCategory()) == normalizeScreenText(category);
}

bool categoryInList(const string& category, const vector<string>& categories) {
    if (categories.empty()) return true;
    string normalizedCategory = normalizeScreenText(category);
    for (const string& item : categories) {
        if (normalizedCategory == normalizeScreenText(item)) return true;
    }
    return false;
}

void drawText(sf::RenderWindow& window, sf::Font& font, const string& value,
              float x, float y, unsigned int size, sf::Color color,
              sf::Text::Style style = sf::Text::Regular) {
    sf::Text text(value, font, size);
    text.setFillColor(color);
    text.setStyle(style);
    text.setPosition(x, y);
    window.draw(text);
}

MapBounds computeBounds(const vector<Point>& points) {
    MapBounds b;
    if (points.empty()) return b;

    b.minX = b.maxX = points[0].getX();
    b.minY = b.maxY = points[0].getY();
    for (const Point& p : points) {
        b.minX = min(b.minX, p.getX());
        b.maxX = max(b.maxX, p.getX());
        b.minY = min(b.minY, p.getY());
        b.maxY = max(b.maxY, p.getY());
    }

    float padX = max(0.001f, (b.maxX - b.minX) * 0.08f);
    float padY = max(0.001f, (b.maxY - b.minY) * 0.08f);
    b.minX -= padX;
    b.maxX += padX;
    b.minY -= padY;
    b.maxY += padY;
    return b;
}

sf::FloatRect mapArea() {
    return sf::FloatRect(SIDEBAR_WIDTH + PADDING,
                         TOP_BAR_HEIGHT + PADDING,
                         WINDOW_WIDTH - SIDEBAR_WIDTH - (PADDING * 2.0f),
                         WINDOW_HEIGHT - TOP_BAR_HEIGHT - (PADDING * 2.0f));
}

sf::Vector2f worldToScreen(float x, float y, const MapBounds& b, const sf::FloatRect& area) {
    float nx = (x - b.minX) / (b.maxX - b.minX);
    float ny = (y - b.minY) / (b.maxY - b.minY);
    return sf::Vector2f(area.left + nx * area.width,
                        area.top + area.height - ny * area.height);
}

sf::Vector2f screenToWorld(float x, float y, const MapBounds& b, const sf::FloatRect& area) {
    float nx = (x - area.left) / area.width;
    float ny = 1.0f - ((y - area.top) / area.height);
    return sf::Vector2f(b.minX + nx * (b.maxX - b.minX),
                        b.minY + ny * (b.maxY - b.minY));
}

void setSourceInputs(const Point& source, vector<TextBox>& inputs,
                     sf::Vector2f& queryPoint) {
    inputs[0].value = source.getName();
    inputs[1].value = source.getCategory();
    inputs[2].value = to_string(source.getX()).substr(0, 7);
    inputs[3].value = to_string(source.getY()).substr(0, 7);
    queryPoint = sf::Vector2f(source.getX(), source.getY());
}

bool chooseSourceNearCoordinates(const vector<Point>& matches, float x, float y,
                                 const sf::FloatRect& area,
                                 const MapBounds& bounds, Point& source) {
    sf::Vector2f selectedScreen = worldToScreen(x, y, bounds, area);
    float bestDistance = RESULT_SELECT_RADIUS;
    bool found = false;

    for (const Point& candidate : matches) {
        sf::Vector2f candidateScreen =
            worldToScreen(candidate.getX(), candidate.getY(), bounds, area);
        float dx = candidateScreen.x - selectedScreen.x;
        float dy = candidateScreen.y - selectedScreen.y;
        float distance = sqrt(dx * dx + dy * dy);
        if (distance < bestDistance) {
            bestDistance = distance;
            source = candidate;
            found = true;
        }
    }

    return found;
}

bool findExistingPointNearInput(const vector<Point>& allPoints, const string& name,
                                const string& category, float x, float y,
                                const sf::FloatRect& area,
                                const MapBounds& bounds, Point& target) {
    vector<Point> matches;
    for (const Point& point : allPoints) {
        if (samePlaceIdentity(point, name, category)) matches.push_back(point);
    }
    return chooseSourceNearCoordinates(matches, x, y, area, bounds, target);
}

bool pointExistsAtCoordinates(const vector<Point>& allPoints, const string& name,
                              const string& category, float x, float y) {
    for (const Point& point : allPoints) {
        if (samePlaceIdentity(point, name, category) &&
            fabs(point.getX() - x) < 0.00001f &&
            fabs(point.getY() - y) < 0.00001f) {
            return true;
        }
    }
    return false;
}

string executableDirectory() {
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (count <= 0) return ".";

    path[count] = '\0';
    string fullPath(path);
    size_t slash = fullPath.find_last_of('/');
    return slash == string::npos ? "." : fullPath.substr(0, slash);
}

string appDataPath(const string& filename) {
    return executableDirectory() + "/" + filename;
}

sf::Color categoryColor(const string& category) {
    static vector<sf::Color> palette = {
        sf::Color(44, 123, 182), sf::Color(215, 48, 39),
        sf::Color(26, 152, 80), sf::Color(253, 174, 97),
        sf::Color(116, 86, 174), sf::Color(102, 194, 165),
        sf::Color(230, 97, 1), sf::Color(94, 60, 153)
    };

    unsigned int hash = 0;
    for (char ch : category) hash = hash * 131u + (unsigned char)ch;
    return palette[hash % palette.size()];
}

vector<string> splitCategories(const string& value) {
    vector<string> result;
    string current;
    for (char ch : value) {
        if (ch == ',') {
            current = trimScreenText(current);
            if (!current.empty()) result.push_back(current);
            current.clear();
        } else {
            current += ch;
        }
    }
    current = trimScreenText(current);
    if (!current.empty()) result.push_back(current);
    return result;
}

void drawButton(sf::RenderWindow& window, sf::Font& font, const Button& b) {
    sf::RectangleShape shape({b.bounds.width, b.bounds.height});
    shape.setPosition(b.bounds.left, b.bounds.top);
    shape.setFillColor(b.selected ? sf::Color(37, 99, 235) : sf::Color(245, 247, 250));
    shape.setOutlineThickness(1.0f);
    shape.setOutlineColor(b.selected ? sf::Color(37, 99, 235) : sf::Color(205, 213, 224));
    window.draw(shape);

    drawText(window, font, b.label, b.bounds.left + 12.0f, b.bounds.top + 8.0f,
             15, b.selected ? sf::Color::White : sf::Color(35, 43, 55), sf::Text::Bold);
}

void drawTextBox(sf::RenderWindow& window, sf::Font& font, const TextBox& box) {
    drawText(window, font, box.label, box.bounds.left, box.bounds.top - 19.0f,
             13, sf::Color(75, 85, 99));

    sf::RectangleShape shape({box.bounds.width, box.bounds.height});
    shape.setPosition(box.bounds.left, box.bounds.top);
    shape.setFillColor(sf::Color::White);
    shape.setOutlineThickness(box.active ? 2.0f : 1.0f);
    shape.setOutlineColor(box.active ? sf::Color(37, 99, 235) : sf::Color(205, 213, 224));
    window.draw(shape);

    string visible = box.value;
    if (visible.size() > 30) visible = visible.substr(visible.size() - 30);
    drawText(window, font, visible, box.bounds.left + 9.0f, box.bounds.top + 7.0f,
             15, sf::Color(31, 41, 55));

    if (box.active) {
        sf::RectangleShape caret({1.5f, 20.0f});
        float textWidth = min((float)visible.size() * 8.5f, box.bounds.width - 20.0f);
        caret.setPosition(box.bounds.left + 10.0f + textWidth, box.bounds.top + 6.0f);
        caret.setFillColor(sf::Color(37, 99, 235));
        window.draw(caret);
    }
}

void drawMapGrid(sf::RenderWindow& window, sf::Font& font, const MapBounds& bounds,
                 const sf::FloatRect& area) {
    sf::RectangleShape background({area.width, area.height});
    background.setPosition(area.left, area.top);
    background.setFillColor(sf::Color(248, 250, 252));
    background.setOutlineThickness(1.0f);
    background.setOutlineColor(sf::Color(203, 213, 225));
    window.draw(background);

    for (int i = 1; i < 6; i++) {
        float x = area.left + area.width * i / 6.0f;
        float y = area.top + area.height * i / 6.0f;

        sf::Vertex vertical[] = {
            sf::Vertex(sf::Vector2f(x, area.top), sf::Color(226, 232, 240)),
            sf::Vertex(sf::Vector2f(x, area.top + area.height), sf::Color(226, 232, 240))
        };
        sf::Vertex horizontal[] = {
            sf::Vertex(sf::Vector2f(area.left, y), sf::Color(226, 232, 240)),
            sf::Vertex(sf::Vector2f(area.left + area.width, y), sf::Color(226, 232, 240))
        };
        window.draw(vertical, 2, sf::Lines);
        window.draw(horizontal, 2, sf::Lines);
    }

    stringstream ss;
    ss << fixed << setprecision(3)
       << "Bounds: X " << bounds.minX << " to " << bounds.maxX
       << "   Y " << bounds.minY << " to " << bounds.maxY;
    drawText(window, font, ss.str(), area.left + 12.0f, area.top + 10.0f,
             13, sf::Color(71, 85, 105));
}

void drawPoints(sf::RenderWindow& window, const vector<Point>& allPoints,
                const vector<Point>& results, const MapBounds& bounds,
                const sf::FloatRect& area) {
    for (const Point& p : allPoints) {
        sf::Vector2f pos = worldToScreen(p.getX(), p.getY(), bounds, area);
        sf::CircleShape dot(POINT_DOT_RADIUS);
        dot.setOrigin(POINT_DOT_RADIUS, POINT_DOT_RADIUS);
        dot.setPosition(pos);
        dot.setFillColor(categoryColor(p.getCategory()));
        window.draw(dot);
    }

    for (const Point& p : results) {
        sf::Vector2f pos = worldToScreen(p.getX(), p.getY(), bounds, area);
        sf::CircleShape ring(RESULT_RING_RADIUS);
        ring.setOrigin(RESULT_RING_RADIUS, RESULT_RING_RADIUS);
        ring.setPosition(pos);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(3.0f);
        ring.setOutlineColor(sf::Color(15, 23, 42));
        window.draw(ring);
    }
}

void applyFeature(const string& mode, RTree& tree, vector<Point>& results,
                  vector<string>& report, vector<TextBox>& inputs,
                  vector<Point>& allPoints, const sf::FloatRect& area,
                  MapBounds& bounds, sf::Vector2f& queryPoint,
                  sf::FloatRect selectedScreenRect, const string& datFile,
                  bool& hasUpdateSelection, Point& selectedUpdatePoint) {
    results.clear();
    report.clear();

    string name = trimScreenText(inputs[0].value);
    string categoryText = trimScreenText(inputs[1].value);
    vector<string> categories = splitCategories(categoryText);
    float x = parseFloatOrDefault(inputs[2].value, queryPoint.x);
    float y = parseFloatOrDefault(inputs[3].value, queryPoint.y);
    int k = parseIntOrDefault(inputs[4].value, mode == "Nearest / KNN" ? 1 : 5);

    if (mode == "All") {
        results = tree.getAllPoints();
        report.push_back("Showing every stored point.");
    } else if (mode == "Name / Category") {
        if (!name.empty()) {
            results = tree.searchByName(name);
            if (!categories.empty()) results = tree.filterByCategory(results, categories);
        } else {
            results = tree.searchByCategory(categories);
        }
        report.push_back("Name search: " + (name.empty() ? "(empty)" : name));
        report.push_back("Category filter: " + (categoryText.empty() ? "(empty)" : categoryText));
        if (results.size() == 1) {
            const Point& selected = results[0];
            setSourceInputs(selected, inputs, queryPoint);
        } else if (results.size() > 1) {
            bool sameCategory = true;
            for (const Point& p : results) {
                if (p.getCategory() != results[0].getCategory()) sameCategory = false;
            }
            if (sameCategory) inputs[1].value = results[0].getCategory();
            report.push_back("Multiple locations found.");
            report.push_back("Click a highlighted dot to select one.");
        }
    } else if (mode == "Nearest / KNN") {
        Point source;
        bool hasSource = false;
        if (!name.empty()) {
            vector<Point> sourceMatches = tree.searchByName(name);
            if (sourceMatches.size() == 1) {
                source = sourceMatches[0];
                hasSource = true;
                x = source.getX();
                y = source.getY();
                setSourceInputs(source, inputs, queryPoint);
            } else if (sourceMatches.empty()) {
                report.push_back("Source name not found.");
            } else if (!inputs[2].value.empty() && !inputs[3].value.empty() &&
                       chooseSourceNearCoordinates(sourceMatches, x, y, area, bounds, source)) {
                hasSource = true;
                x = source.getX();
                y = source.getY();
                setSourceInputs(source, inputs, queryPoint);
            } else {
                results = sourceMatches;
                report.push_back("Multiple source locations found.");
                report.push_back("Click one highlighted dot first.");
            }
        } else if (!inputs[2].value.empty() && !inputs[3].value.empty()) {
            hasSource = true;
        } else {
            report.push_back("Enter a name or coordinates first.");
        }

        if (hasSource) {
            vector<Point> candidates = categories.empty()
                ? tree.k_nearestNeighbors(x, y, k + 1)
                : tree.k_nearestNeighborsInCategories(x, y, k + 1, categories);
            for (const Point& candidate : candidates) {
                if (name.empty() || !sameScreenPoint(candidate, source)) {
                    results.push_back(candidate);
                    if ((int)results.size() == k) break;
                }
            }
            report.push_back(k == 1 ? "Nearest neighbor search"
                                    : "K-nearest search with K = " + to_string(k));
            report.push_back(categories.empty()
                             ? "Category filter: all categories"
                             : "Category filter: " + categoryText);
        }
    } else if (mode == "Insert") {
        float insertX = 0.0f;
        float insertY = 0.0f;
        if (name.empty() || categoryText.empty() ||
            !parseFloatStrict(inputs[2].value, insertX) ||
            !parseFloatStrict(inputs[3].value, insertY)) {
            report.push_back("Enter name, category, X, and Y to insert.");
        } else {
            Point inserted(insertX, insertY, name, categoryText);
            if (pointExistsAtCoordinates(allPoints, name, categoryText, insertX, insertY)) {
                report.push_back("That location already exists.");
            } else {
                tree.insert(inserted);
                tree.saveTreeToFile(datFile);
                allPoints = tree.getAllPoints();
                bounds = computeBounds(allPoints);
                results.push_back(inserted);
                setSourceInputs(inserted, inputs, queryPoint);
                report.push_back("Inserted location and saved dat file.");
            }
        }
    } else if (mode == "Delete") {
        if (name.empty()) {
            report.push_back("Enter a name to delete.");
        } else {
            vector<Point> matches = tree.searchByName(name);
            if (!categories.empty()) matches = tree.filterByCategory(matches, categories);

            float deleteX = 0.0f;
            float deleteY = 0.0f;
            bool hasDeletePoint = parseFloatStrict(inputs[2].value, deleteX) &&
                                  parseFloatStrict(inputs[3].value, deleteY);
            Point target;

            if (matches.empty()) {
                report.push_back("No matching location found for: " + name);
            } else if (matches.size() == 1 && !hasDeletePoint) {
                target = matches[0];
                hasDeletePoint = true;
            } else if (hasDeletePoint &&
                       chooseSourceNearCoordinates(matches, deleteX, deleteY, area, bounds, target)) {
                hasDeletePoint = true;
            } else {
                results = matches;
                report.push_back("Multiple locations found for delete.");
                report.push_back("Click one highlighted dot to delete it.");
                hasDeletePoint = false;
            }

            if (hasDeletePoint) {
                string deletedName = target.getName();
                if (!tree.deletePoint(target)) {
                    report.push_back("Could not delete selected location.");
                } else {
                    tree.saveTreeToFile(datFile);
                    allPoints = tree.getAllPoints();
                    bounds = computeBounds(allPoints);
                    selectedScreenRect = sf::FloatRect();
                    queryPoint = sf::Vector2f((bounds.minX + bounds.maxX) / 2.0f,
                                              (bounds.minY + bounds.maxY) / 2.0f);
                    for (TextBox& input : inputs) input.value.clear();
                    report.push_back("Deleted selected location: " + deletedName);
                    report.push_back("Saved dat file.");
                }
            }
        }
    } else if (mode == "Update") {
        if (!hasUpdateSelection) {
            if (name.empty()) {
                report.push_back("Enter the previous name to update.");
            } else {
                vector<Point> matches = tree.searchByName(name);
                if (!categories.empty()) matches = tree.filterByCategory(matches, categories);

                float selectedX = 0.0f;
                float selectedY = 0.0f;
                bool hasSelectedPoint = parseFloatStrict(inputs[2].value, selectedX) &&
                                        parseFloatStrict(inputs[3].value, selectedY);

                if (matches.empty()) {
                    report.push_back("No matching location found for: " + name);
                } else if (matches.size() == 1 && !hasSelectedPoint) {
                    selectedUpdatePoint = matches[0];
                    hasUpdateSelection = true;
                    setSourceInputs(selectedUpdatePoint, inputs, queryPoint);
                    report.push_back("Selected location for update.");
                    report.push_back("Edit the name field, then click Run.");
                } else if (hasSelectedPoint &&
                           chooseSourceNearCoordinates(matches, selectedX, selectedY,
                                                       area, bounds, selectedUpdatePoint)) {
                    hasUpdateSelection = true;
                    setSourceInputs(selectedUpdatePoint, inputs, queryPoint);
                    report.push_back("Selected location for update.");
                    report.push_back("Edit the name field, then click Run.");
                } else {
                    results = matches;
                    report.push_back("Multiple locations found for update.");
                    report.push_back("Click one highlighted dot to select it.");
                }
            }
        } else {
            string newName = name;
            if (newName.empty()) {
                report.push_back("Enter the new name.");
            } else {
                Point updated(selectedUpdatePoint.getX(), selectedUpdatePoint.getY(),
                              newName, selectedUpdatePoint.getCategory());
                if (!tree.updatePoint(selectedUpdatePoint, updated)) {
                    report.push_back("Could not update selected location.");
                } else {
                    tree.saveTreeToFile(datFile);
                    allPoints = tree.getAllPoints();
                    bounds = computeBounds(allPoints);
                    results.push_back(updated);
                    setSourceInputs(updated, inputs, queryPoint);
                    hasUpdateSelection = false;
                    report.push_back("Updated selected location name.");
                    report.push_back("Saved dat file.");
                }
            }
        }
    } else if (mode == "Range") {
        if (selectedScreenRect.width < 4.0f || selectedScreenRect.height < 4.0f) {
            report.push_back("Drag a rectangle on the map first.");
        } else {
            sf::Vector2f a = screenToWorld(selectedScreenRect.left, selectedScreenRect.top,
                                           bounds, area);
            sf::Vector2f b = screenToWorld(selectedScreenRect.left + selectedScreenRect.width,
                                           selectedScreenRect.top + selectedScreenRect.height,
                                           bounds, area);
            Rectangle r(min(a.x, b.x), min(a.y, b.y), max(a.x, b.x), max(a.y, b.y));
            results = categories.empty() ? tree.search(r) : tree.searchByCategoryInRange(r, categories);
            report.push_back("Range search from the dragged rectangle.");
        }
    } else if (mode == "Placement") {
        string category = categories.empty() ? "Healthcare" : categories[0];
        vector<Point> all = tree.getAllPoints();
        vector<Point> sameCategory = tree.searchByCategory({category});

        if (all.empty() || sameCategory.empty()) {
            report.push_back("No existing data for: " + category);
        } else {
            float bestScore = -1.0f;
            float bestGap = 0.0f;
            float bestActivityDistance = 0.0f;
            Point suggestion;
            int gridSize = 28;

            for (int row = 2; row < gridSize - 2; row++) {
                for (int col = 2; col < gridSize - 2; col++) {
                    float cx = bounds.minX + (bounds.maxX - bounds.minX) * col / (float)(gridSize - 1);
                    float cy = bounds.minY + (bounds.maxY - bounds.minY) * row / (float)(gridSize - 1);

                    float nearestSame = 999999.0f;
                    for (const Point& p : sameCategory) {
                        nearestSame = min(nearestSame, pointDistance(cx, cy, p.getX(), p.getY()));
                    }

                    float nearestAny = 999999.0f;
                    for (const Point& p : all) {
                        nearestAny = min(nearestAny, pointDistance(cx, cy, p.getX(), p.getY()));
                    }

                    float score = nearestSame - (nearestAny * 0.45f);
                    if (score > bestScore) {
                        bestScore = score;
                        bestGap = nearestSame;
                        bestActivityDistance = nearestAny;
                        suggestion = Point(cx, cy, "Suggested Location", category);
                    }
                }
            }

            results.push_back(suggestion);
            stringstream ssGap;
            ssGap << fixed << setprecision(4) << "Gap from nearest " << category << ": " << bestGap;
            stringstream ssActivity;
            ssActivity << fixed << setprecision(4) << "Distance to nearest existing place: " << bestActivityDistance;
            report.push_back("Best placement suggestion for: " + category);
            report.push_back(ssGap.str());
            report.push_back(ssActivity.str());
        }
    } else if (mode == "Ranking") {
        vector<Point> all = tree.getAllPoints();
        if (all.empty()) {
            report.push_back("No data loaded.");
        } else {
            MapBounds b = computeBounds(all);
            float midX = (b.minX + b.maxX) / 2.0f;
            float midY = (b.minY + b.maxY) / 2.0f;
            vector<pair<Rectangle, string>> zones = {
                {Rectangle(b.minX, midY, midX, b.maxY), "NW"},
                {Rectangle(midX, midY, b.maxX, b.maxY), "NE"},
                {Rectangle(b.minX, b.minY, midX, midY), "SW"},
                {Rectangle(midX, b.minY, b.maxX, midY), "SE"}
            };
            RegionMetrics best = findBestRegion(tree, zones, "overall");
            results = tree.search(best.region);
            stringstream ss;
            ss << fixed << setprecision(2) << "Best quadrant: " << best.regionName
               << "  score " << best.totalScore;
            report.push_back(ss.str());
            report.push_back("Points: " + to_string(best.pointCount) +
                             "  unique categories: " + to_string(best.uniqueCategories));
        }
    } else if (mode == "Coverage") {
        CoverageConfig cfg;
        if (categories.empty()) {
            cfg.addCategory("Healthcare", 0.05);
            cfg.addCategory("Restaurant", 0.05);
        } else {
            for (const string& c : categories) cfg.addCategory(c, 0.05);
        }

        CoverageAnalyzer analyzer(tree);
        Rectangle region(bounds.minX, bounds.minY, bounds.maxX, bounds.maxY);
        CoverageResult coverage = analyzer.analyze(region, cfg, 8, 8);
        report.push_back("Coverage grid: 8 x 8");
        for (int i = 0; i < cfg.size(); i++) {
            stringstream ss;
            ss << fixed << setprecision(1) << cfg.rules[i].category << ": "
               << coverage.categoryCoveragePct(i) << "% covered";
            report.push_back(ss.str());
        }
        report.push_back("Underserved cells: " + to_string((int)coverage.underservedCells.size()));
    }

    report.push_back("Results: " + to_string((int)results.size()));
    int limit = min((int)results.size(), 8);
    for (int i = 0; i < limit; i++) {
        report.push_back(to_string(i + 1) + ". " + pointLine(results[i]));
    }
}

int main() {
    RTree tree;
    const string csvFile = appDataPath("cleaned_locations.csv");
    const string datFile = appDataPath("rtree_data.dat");

    if (tree.treeFileExists(datFile)) {
        tree.loadTreeFromFile(datFile);
    } else {
        loadCSVIntoTree(csvFile, tree);
        tree.saveTreeToFile(datFile);
    }

    vector<Point> allPoints = tree.getAllPoints();
    MapBounds bounds = computeBounds(allPoints);

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
                            "R-Tree Spatial Search UI");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") &&
        !font.loadFromFile("arial.ttf")) {
        return 1;
    }

    vector<Button> buttons;
    vector<string> modes = {"All", "Name / Category", "Range", "Nearest / KNN",
                            "Insert", "Delete", "Update", "Placement", "Ranking",
                            "Coverage"};
    float buttonY = 86.0f;
    for (size_t i = 0; i < modes.size(); i++) {
        buttons.push_back({sf::FloatRect(18.0f, buttonY + i * 40.0f, 292.0f, 32.0f),
                           modes[i], i == 0});
    }

    vector<TextBox> inputs = {
        {sf::FloatRect(18.0f, 508.0f, 292.0f, 32.0f), "Name contains", ""},
        {sf::FloatRect(18.0f, 566.0f, 292.0f, 32.0f), "Categories, comma separated", ""},
        {sf::FloatRect(18.0f, 624.0f, 138.0f, 32.0f), "X / latitude", ""},
        {sf::FloatRect(172.0f, 624.0f, 138.0f, 32.0f), "Y / longitude", ""},
        {sf::FloatRect(18.0f, 682.0f, 138.0f, 32.0f), "K value", ""}
    };

    Button runButton{sf::FloatRect(172.0f, 682.0f, 64.0f, 32.0f), "Run", false};
    Button clearButton{sf::FloatRect(246.0f, 682.0f, 64.0f, 32.0f), "Clear", false};

    string mode = "All";
    vector<Point> results = allPoints;
    vector<string> report = {"Showing every stored point.",
                             "Results: " + to_string((int)results.size())};

    sf::FloatRect area = mapArea();
    bool draggingRange = false;
    sf::Vector2f dragStart;
    sf::FloatRect selectedRect;
    bool hasUpdateSelection = false;
    Point selectedUpdatePoint;
    sf::Vector2f queryPoint((bounds.minX + bounds.maxX) / 2.0f,
                            (bounds.minY + bounds.maxY) / 2.0f);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mouse((float)event.mouseButton.x, (float)event.mouseButton.y);

                bool clickedInput = false;
                for (int i = 0; i < (int)inputs.size(); i++) {
                    bool clickedThisInput = inputs[i].bounds.contains(mouse);
                    inputs[i].active = clickedThisInput && isVisibleTextBox(i) &&
                                       isEditableTextBox(i, mode);
                    clickedInput = clickedInput || (clickedThisInput && isVisibleTextBox(i));
                }

                for (Button& button : buttons) {
                    if (button.bounds.contains(mouse)) {
                        mode = button.label;
                        for (Button& other : buttons) other.selected = false;
                        button.selected = true;
                        for (TextBox& input : inputs) {
                            input.value.clear();
                            input.active = false;
                        }
                        selectedRect = sf::FloatRect();
                        hasUpdateSelection = false;
                        queryPoint = sf::Vector2f((bounds.minX + bounds.maxX) / 2.0f,
                                                  (bounds.minY + bounds.maxY) / 2.0f);

                        report.clear();
                        if (mode == "All") {
                            results = allPoints;
                            report.push_back("Showing every stored point.");
                            report.push_back("Results: " + to_string((int)results.size()));
                        } else {
                            results.clear();
                            report.push_back(mode + " ready.");
                            report.push_back("Enter values or click a point.");
                        }
                    }
                }

                if (runButton.bounds.contains(mouse)) {
                    applyFeature(mode, tree, results, report, inputs, allPoints, area,
                                 bounds, queryPoint, selectedRect, datFile,
                                 hasUpdateSelection, selectedUpdatePoint);
                }

                if (clearButton.bounds.contains(mouse)) {
                    for (TextBox& input : inputs) {
                        input.value.clear();
                        input.active = false;
                    }
                    draggingRange = false;
                    selectedRect = sf::FloatRect();
                    hasUpdateSelection = false;
                    queryPoint = sf::Vector2f((bounds.minX + bounds.maxX) / 2.0f,
                                              (bounds.minY + bounds.maxY) / 2.0f);
                    results.clear();
                    report.clear();
                    report.push_back("Fields cleared.");
                }

                if (!clickedInput && area.contains(mouse)) {
                    sf::Vector2f world = screenToWorld(mouse.x, mouse.y, bounds, area);
                    queryPoint = world;
                    inputs[2].value = to_string(world.x).substr(0, 7);
                    inputs[3].value = to_string(world.y).substr(0, 7);

                    float bestPixelDistance = RESULT_SELECT_RADIUS;
                    Point clickedPlace;
                    bool foundClickedPlace = false;

                    for (const Point& point : results) {
                        sf::Vector2f pointScreen = worldToScreen(point.getX(), point.getY(), bounds, area);
                        float dx = pointScreen.x - mouse.x;
                        float dy = pointScreen.y - mouse.y;
                        float distance = sqrt(dx * dx + dy * dy);
                        if (distance < bestPixelDistance) {
                            bestPixelDistance = distance;
                            clickedPlace = point;
                            foundClickedPlace = true;
                        }
                    }

                    if (!foundClickedPlace) {
                        bestPixelDistance = POINT_SELECT_RADIUS;
                        for (const Point& point : allPoints) {
                            sf::Vector2f pointScreen = worldToScreen(point.getX(), point.getY(), bounds, area);
                            float dx = pointScreen.x - mouse.x;
                            float dy = pointScreen.y - mouse.y;
                            float distance = sqrt(dx * dx + dy * dy);
                            if (distance < bestPixelDistance) {
                                bestPixelDistance = distance;
                                clickedPlace = point;
                                foundClickedPlace = true;
                            }
                        }
                    }

                    if (foundClickedPlace && mode != "Range") {
                        inputs[0].value = clickedPlace.getName();
                        inputs[1].value = clickedPlace.getCategory();
                        inputs[2].value = to_string(clickedPlace.getX()).substr(0, 7);
                        inputs[3].value = to_string(clickedPlace.getY()).substr(0, 7);
                        queryPoint = sf::Vector2f(clickedPlace.getX(), clickedPlace.getY());
                    }

                    if (mode == "Range") {
                        draggingRange = true;
                        dragStart = mouse;
                        selectedRect = sf::FloatRect(mouse.x, mouse.y, 0, 0);
                    } else if (mode == "Nearest / KNN" || mode == "Delete" || mode == "Update") {
                        applyFeature(mode, tree, results, report, inputs, allPoints, area,
                                     bounds, queryPoint, selectedRect, datFile,
                                     hasUpdateSelection, selectedUpdatePoint);
                    }
                }
            }

            if (event.type == sf::Event::MouseMoved && draggingRange) {
                sf::Vector2f mouse((float)event.mouseMove.x, (float)event.mouseMove.y);
                mouse.x = min(max(mouse.x, area.left), area.left + area.width);
                mouse.y = min(max(mouse.y, area.top), area.top + area.height);
                selectedRect.left = min(dragStart.x, mouse.x);
                selectedRect.top = min(dragStart.y, mouse.y);
                selectedRect.width = fabs(mouse.x - dragStart.x);
                selectedRect.height = fabs(mouse.y - dragStart.y);
            }

            if (event.type == sf::Event::MouseButtonReleased &&
                event.mouseButton.button == sf::Mouse::Left && draggingRange) {
                draggingRange = false;
                applyFeature(mode, tree, results, report, inputs, allPoints, area,
                             bounds, queryPoint, selectedRect, datFile,
                             hasUpdateSelection, selectedUpdatePoint);
            }

            if (event.type == sf::Event::TextEntered) {
                int activeIndex = activeTextBoxIndex(inputs);
                if (activeIndex >= 0 &&
                    isEditableTextBox(activeIndex, mode) &&
                    event.text.unicode >= 32 &&
                    event.text.unicode < 127 &&
                    inputs[activeIndex].value.size() < 80) {
                    inputs[activeIndex].value += (char)event.text.unicode;
                }
            }

            if (event.type == sf::Event::KeyPressed) {
                int activeIndex = activeTextBoxIndex(inputs);

                if (activeIndex >= 0 && isEditableTextBox(activeIndex, mode) &&
                    event.key.code == sf::Keyboard::BackSpace) {
                    if (!inputs[activeIndex].value.empty()) inputs[activeIndex].value.pop_back();
                } else if (activeIndex >= 0 && isEditableTextBox(activeIndex, mode) &&
                           event.key.code == sf::Keyboard::Delete) {
                    inputs[activeIndex].value.clear();
                } else if (event.key.code == sf::Keyboard::Tab) {
                    int nextIndex = nextEditableTextBoxIndex(activeIndex, (int)inputs.size(), mode);
                    for (TextBox& input : inputs) input.active = false;
                    if (nextIndex >= 0) inputs[nextIndex].active = true;
                } else if (event.key.code == sf::Keyboard::Escape) {
                    for (TextBox& input : inputs) input.active = false;
                } else if (event.key.code == sf::Keyboard::Enter) {
                    applyFeature(mode, tree, results, report, inputs, allPoints, area,
                                 bounds, queryPoint, selectedRect, datFile,
                                 hasUpdateSelection, selectedUpdatePoint);
                }
            }
        }

        window.clear(sf::Color(241, 245, 249));

        sf::RectangleShape topBar({(float)WINDOW_WIDTH, TOP_BAR_HEIGHT});
        topBar.setFillColor(sf::Color(15, 23, 42));
        window.draw(topBar);
        drawText(window, font, "R-Tree Spatial Search", 22, 18, 26,
                 sf::Color::White, sf::Text::Bold);
        drawText(window, font, "Interactive screen for range, nearest, ranking, coverage, and category queries",
                 360, 25, 15, sf::Color(203, 213, 225));

        sf::RectangleShape side({SIDEBAR_WIDTH, (float)WINDOW_HEIGHT - TOP_BAR_HEIGHT});
        side.setPosition(0, TOP_BAR_HEIGHT);
        side.setFillColor(sf::Color::White);
        window.draw(side);

        for (const Button& button : buttons) drawButton(window, font, button);
        for (int i = 0; i < (int)inputs.size(); i++) {
            if (isVisibleTextBox(i)) drawTextBox(window, font, inputs[i]);
        }
        drawButton(window, font, runButton);
        drawButton(window, font, clearButton);

        drawMapGrid(window, font, bounds, area);
        drawPoints(window, allPoints, results, bounds, area);

        sf::Vector2f queryScreen = worldToScreen(queryPoint.x, queryPoint.y, bounds, area);
        sf::CircleShape target(6.0f);
        target.setOrigin(6.0f, 6.0f);
        target.setPosition(queryScreen);
        target.setFillColor(sf::Color::White);
        target.setOutlineThickness(2.0f);
        target.setOutlineColor(sf::Color(220, 38, 38));
        window.draw(target);

        if (selectedRect.width > 0 && selectedRect.height > 0) {
            sf::RectangleShape selection({selectedRect.width, selectedRect.height});
            selection.setPosition(selectedRect.left, selectedRect.top);
            selection.setFillColor(sf::Color(37, 99, 235, 35));
            selection.setOutlineThickness(2.0f);
            selection.setOutlineColor(sf::Color(37, 99, 235));
            window.draw(selection);
        }

        sf::FloatRect panel(area.left + area.width - 360.0f, area.top + 18.0f, 342.0f, 270.0f);
        sf::RectangleShape reportBox({panel.width, panel.height});
        reportBox.setPosition(panel.left, panel.top);
        reportBox.setFillColor(sf::Color(255, 255, 255, 235));
        reportBox.setOutlineThickness(1.0f);
        reportBox.setOutlineColor(sf::Color(203, 213, 225));
        window.draw(reportBox);

        drawText(window, font, mode + " Output", panel.left + 14.0f, panel.top + 12.0f,
                 16, sf::Color(15, 23, 42), sf::Text::Bold);
        float lineY = panel.top + 44.0f;
        int visibleLines = min((int)report.size(), 10);
        for (int i = 0; i < visibleLines; i++) {
            string line = report[i];
            if (line.size() > 43) line = line.substr(0, 40) + "...";
            drawText(window, font, line, panel.left + 14.0f, lineY + i * 20.0f,
                     13, sf::Color(51, 65, 85));
        }

        window.display();
    }

    return 0;
}
