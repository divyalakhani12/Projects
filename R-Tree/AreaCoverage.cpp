#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;
#include "rtree.cpp"


// ===================== HELPERS =====================

string normalize(string s) {
    int start = 0;
    while (start < (int)s.size() && isspace(s[start])) start++;
    s = s.substr(start);
    while (!s.empty() && isspace(s.back())) s.pop_back();
    for (int i = 0; i < (int)s.size(); i++)
        s[i] = tolower(s[i]);
    return s;
}

// ===================== CONFIG =====================

struct Rule {
    string category;
    double radius;
};

class CoverageConfig {
public:
    vector<Rule> rules;

    void addCategory(string c, double r) {
        Rule rule;
        rule.category = c;
        rule.radius   = r;
        rules.push_back(rule);
    }

    int size() const {
        return (int)rules.size();
    }

    void print() const {
        for (int i = 0; i < (int)rules.size(); i++)
            cout << rules[i].category << " -> radius = "
                 << rules[i].radius << "\n";
    }
};

// ===================== GRID CELL =====================

struct GridCell {
    int    row, col;
    double centerX, centerY;

    vector<bool>   covered;
    vector<string> missingCategories;

    bool isFullyCovered() const {
        for (int i = 0; i < (int)covered.size(); i++)
            if (!covered[i]) return false;
        return true;
    }
};

// ===================== RESULT =====================

struct CoverageResult {
    int rows      = 0;
    int cols      = 0;
    int totalCells = 0;

    vector<int>      coveredCountPerCategory;
    vector<GridCell> underservedCells;

    double categoryCoveragePct(int i) const {
        if (totalCells == 0) return 0.0;
        return 100.0 * coveredCountPerCategory[i] / totalCells;
    }

    double overallCoveragePct(int numCategories) const {
        if (numCategories == 0) return 0.0;
        double sum = 0.0;
        for (int i = 0; i < numCategories; i++)
            sum += categoryCoveragePct(i);
        return sum / numCategories;
    }

    void printSummary(const CoverageConfig& cfg) const {
        cout << "\n==========================================\n";
        cout << "     AREA COVERAGE ANALYSIS REPORT\n";
        cout << "==========================================\n";
        cout << "Grid : " << rows << " rows x " << cols
             << " cols = " << totalCells << " total cells\n\n";

        cout << "Per-Category Coverage:\n";
        cout << "------------------------------------------\n";

        for (int i = 0; i < cfg.size(); i++) {
            double pct = categoryCoveragePct(i);
            int    bar = (int)(pct / 5);

            string filled = "";
            string empty  = "";
            for (int b = 0; b < bar;  b++) filled += "#";
            for (int b = bar; b < 20; b++) empty  += "-";

            cout << "  " << setw(24) << left << cfg.rules[i].category
                 << " [" << filled << empty << "] "
                 << fixed << setprecision(1) << pct << "%\n";
        }

        cout << "\nOverall Coverage Score : "
             << fixed << setprecision(1)
             << overallCoveragePct(cfg.size()) << "%\n";

        cout << "\nUnderserved Cells      : "
             << underservedCells.size() << " / " << totalCells << "\n";

        if (!underservedCells.empty()) {
            cout << "\nWorst underserved locations (most missing services):\n";

            // index array, selection-sort by missing count descending
            vector<int> order;
            for (int i = 0; i < (int)underservedCells.size(); i++)
                order.push_back(i);

            for (int i = 0; i < (int)order.size() - 1; i++) {
                for (int j = i + 1; j < (int)order.size(); j++) {
                    int mi = (int)underservedCells[order[i]].missingCategories.size();
                    int mj = (int)underservedCells[order[j]].missingCategories.size();
                    if (mj > mi) {
                        int tmp  = order[i];
                        order[i] = order[j];
                        order[j] = tmp;
                    }
                }
            }

            int shown = 0;
            for (int i = 0; i < (int)order.size() && shown < 8; i++, shown++) {
                const GridCell& cell = underservedCells[order[i]];
                cout << "  Cell [r" << cell.row << ",c" << cell.col << "]"
                     << " centre=(" << fixed << setprecision(4)
                     << cell.centerX << ", " << cell.centerY << ")"
                     << "  missing: ";
                for (int m = 0; m < (int)cell.missingCategories.size(); m++) {
                    if (m > 0) cout << ", ";
                    cout << cell.missingCategories[m];
                }
                cout << "\n";
            }
        }
        cout << "==========================================\n\n";
    }
};

// ===================== ANALYZER =====================

class CoverageAnalyzer {
private:
    RTree& rtree;

    double distance(double x1, double y1,
                    double x2, double y2) const {
        double dx = x1 - x2;
        double dy = y1 - y2;
        return sqrt(dx * dx + dy * dy);
    }

    bool isCovered(double cx, double cy,
                   const string& category,
                   double radius) const {
        Rectangle searchBox(
            (float)(cx - radius), (float)(cy - radius),
            (float)(cx + radius), (float)(cy + radius)
        );

        vector<Point> candidates = rtree.search(searchBox);

        for (int i = 0; i < (int)candidates.size(); i++) {
            if (normalize(candidates[i].getCategory()) == normalize(category)) {
                double d = distance(cx, cy,
                                    candidates[i].getX(),
                                    candidates[i].getY());
                if (d <= radius)
                    return true;
            }
        }
        return false;
    }

public:
    CoverageAnalyzer(RTree& tree) : rtree(tree) {}

    CoverageResult analyze(const Rectangle& region,
                           const CoverageConfig& cfg,
                           int rows, int cols) const {
        CoverageResult result;
        result.rows       = rows;
        result.cols       = cols;
        result.totalCells = rows * cols;

        for (int i = 0; i < cfg.size(); i++)
            result.coveredCountPerCategory.push_back(0);

        double cellW = (region.getMaxX() - region.getMinX()) / cols;
        double cellH = (region.getMaxY() - region.getMinY()) / rows;

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                GridCell cell;
                cell.row     = r;
                cell.col     = c;
                cell.centerX = region.getMinX() + r * cellW + cellW / 2.0;
                cell.centerY = region.getMinY() + c * cellH + cellH / 2.0;

                for (int i = 0; i < cfg.size(); i++) {
                    bool cov = isCovered(cell.centerX, cell.centerY,
                                         cfg.rules[i].category,
                                         cfg.rules[i].radius);
                    cell.covered.push_back(cov);

                    if (cov)
                        result.coveredCountPerCategory[i]++;
                    else
                        cell.missingCategories.push_back(cfg.rules[i].category);
                }

                if (!cell.isFullyCovered())
                    result.underservedCells.push_back(cell);
            }
        }

        return result;
    }

    void printCoverageAt(double px, double py,
                         const CoverageConfig& cfg) const {
        cout << "Coverage check at (" << fixed << setprecision(4)
             << px << ", " << py << "):\n";

        bool allCovered = true;
        for (int i = 0; i < cfg.size(); i++) {
            bool cov = isCovered(px, py,
                                  cfg.rules[i].category,
                                  cfg.rules[i].radius);
            cout << "  " << setw(24) << left << cfg.rules[i].category
                 << ": " << (cov ? "COVERED" : "NOT COVERED") << "\n";
            if (!cov) allCovered = false;
        }
        cout << "  => Fully covered: " << (allCovered ? "YES" : "NO") << "\n\n";
    }
};