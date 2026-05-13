#ifndef REGION_RANKING_CPP
#define REGION_RANKING_CPP

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include "rtree.cpp"

using namespace std;


// ╔══════════════════════════════════════════════════════════════╗
//   RegionMetrics — report card for one rectangular zone
//
//   Diversity metric:
//     Category Ratio = uniqueCategories / pointCount
//     Simple, explainable — "how many categories per point?"
//     Higher ratio = more variety relative to size.
// ╚══════════════════════════════════════════════════════════════╝
struct RegionMetrics {
    Rectangle region;
    string    regionName;

    // ── Basic metrics ──────────────────────────────────────────
    int   pointCount;
    float density;           // points per unit area

    // ── Category metrics ───────────────────────────────────────
    int              uniqueCategories;
    map<string, int> categoryDistribution;

    // ── Diversity metric ───────────────────────────────────────
    // categoryRatio = uniqueCategories / pointCount
    // Ranges from near-0 (many points, few categories)
    // to 1.0 (every point is a different category).
    float categoryRatio;

    // ── Composite score (set by RegionRanker after normalising) ─
    float totalScore;

    RegionMetrics()
        : pointCount(0), density(0.0f), uniqueCategories(0),
          categoryRatio(0.0f), totalScore(0.0f) {}

    // density = pointCount / area
    void calculateDensity() {
        float area = region.area();
        density = (area > 0.0f) ? (float)pointCount / area : 0.0f;
    }

    // categoryRatio = uniqueCategories / pointCount
    // Example: 5 categories, 87 points  → 0.057
    //          5 categories, 10 points  → 0.500  (more spread out)
    void calculateCategoryRatio() {
        categoryRatio = (pointCount > 0)
                        ? (float)uniqueCategories / (float)pointCount
                        : 0.0f;
    }

    // Preliminary score — overwritten by RegionRanker::normalizeScores()
    void calculateCompositeScore(float densityWeight  = 0.35f,
                                  float categoryWeight = 0.35f,
                                  float ratioWeight    = 0.30f) {
        totalScore = (density                 * densityWeight)
                   + (uniqueCategories        * categoryWeight)
                   + (categoryRatio * 100.0f  * ratioWeight);
    }

    void print() const {
        cout << "\n+-----------------------------------------------------+\n";
        cout << "| Region: " << regionName;
        for (int i = (int)regionName.length(); i < 43; i++) cout << " ";
        cout << "|\n";
        cout << "+-----------------------------------------------------+\n";
        cout << "| Bounds: [" << region.getMinX() << ", " << region.getMinY()
            << "] to [" << region.getMaxX() << ", " << region.getMaxY() << "]\n";
        cout << "+-----------------------------------------------------+\n";
        printf("| Points:              %-30d |\n", pointCount);
        printf("| Density:             %-30.4f |\n", density);
        printf("| Unique Categories:   %-30d |\n", uniqueCategories);
        printf("| Category Ratio:      %-30.4f |\n", categoryRatio);
        cout << "+-----------------------------------------------------+\n";
        printf("| COMPOSITE SCORE:     %-30.2f |\n", totalScore);
        cout << "+-----------------------------------------------------+\n";

        if (!categoryDistribution.empty()) {
            cout << "\n  Category Distribution:\n";
            vector<pair<string, int>> sorted(
                categoryDistribution.begin(), categoryDistribution.end());
            sort(sorted.begin(), sorted.end(),
                [](const pair<string, int>& a, const pair<string, int>& b) {
                    return a.second > b.second;
                });
            for (const auto& [cat, count] : sorted) {
                float pct = (pointCount > 0)
                            ? (100.0f * count / pointCount)
                            : 0.0f;
                printf("    * %-20s : %4d (%.1f%%)\n",
                    cat.c_str(), count, pct);
            }
        }
    }
};


// ╔══════════════════════════════════════════════════════════════╗
//   RegionRanker — normalises and sorts a collection of regions
// ╚══════════════════════════════════════════════════════════════╝
class RegionRanker {
private:
    vector<RegionMetrics> regions;

    // ════════════════════════════════════════════════════════════
    //  NORMALISATION
    //  Each raw metric is scaled to [0, 100] by dividing by its
    //  maximum observed value, then weighted and summed.
    // ════════════════════════════════════════════════════════════
    void normalizeScores() {
        float maxDensity = 0.0f;
        float maxRatio   = 0.0f;
        int   maxUnique  = 0;

        for (const auto& r : regions) {
            if (r.density          > maxDensity) maxDensity = r.density;
            if (r.categoryRatio    > maxRatio)   maxRatio   = r.categoryRatio;
            if (r.uniqueCategories > maxUnique)  maxUnique  = r.uniqueCategories;
        }

        // Guard against all-zero columns
        if (maxDensity == 0.0f) maxDensity = 1.0f;
        if (maxRatio   == 0.0f) maxRatio   = 1.0f;
        if (maxUnique  == 0)    maxUnique  = 1;

        for (auto& r : regions) {
            float normDensity = (r.density                        / maxDensity) * 100.0f;
            float normUnique  = ((float)r.uniqueCategories        / maxUnique)  * 100.0f;
            float normRatio   = (r.categoryRatio                  / maxRatio)   * 100.0f;

            r.totalScore = normDensity * 0.35f
                         + normUnique  * 0.35f
                         + normRatio   * 0.30f;
        }
    }

public:
    void addRegion(const RegionMetrics& m) { regions.push_back(m); }

    // Normalise then sort descending by totalScore
    void rankRegions() {
        normalizeScores();
        sort(regions.begin(), regions.end(),
             [](const RegionMetrics& a, const RegionMetrics& b) {
                 return a.totalScore > b.totalScore;
             });
    }

    void printRankings() const {
        cout << "\n+=====================================================+\n";
        cout << "|           REGION RANKING RESULTS                    |\n";
        cout << "+=====================================================+\n";
        for (int i = 0; i < (int)regions.size(); i++) {
            cout << "\n  RANK #" << (i + 1) << "\n";
            regions[i].print();
        }
        cout << "\n+=====================================================+\n";
        cout << "|              RANKING SUMMARY                        |\n";
        cout << "+=====================================================+\n\n";
        printf("  %-22s | Score  | Points | Density     | Categories | CatRatio\n",
            "Region");
        cout << "  " << string(80, '-') << "\n";
        for (const auto& r : regions)
            printf("  %-22s | %6.2f | %6d | %11.4f | %10d | %8.4f\n",
                r.regionName.c_str(), r.totalScore, r.pointCount,
                r.density, r.uniqueCategories, r.categoryRatio);
        cout << "\n";
    }

    RegionMetrics getTopRegion() const {
        return regions.empty() ? RegionMetrics() : regions[0];
    }

    vector<RegionMetrics> getRankings() const { return regions; }
};


// ════════════════════════════════════════════════════════════════
//  SINGLE REGION ANALYSIS
//
//  Fetches all points inside the rectangle from the R-Tree,
//  builds the category distribution, and computes all metrics.
// ════════════════════════════════════════════════════════════════
RegionMetrics analyzeRegion(RTree& tree,
                             const Rectangle& region,
                             const string& regionName) {
    RegionMetrics metrics;
    metrics.region     = region;
    metrics.regionName = regionName;

    vector<Point> pts  = tree.search(region);
    metrics.pointCount = (int)pts.size();

    set<string> uniqueCats;
    for (const auto& p : pts) {
        uniqueCats.insert(p.getCategory());
        metrics.categoryDistribution[p.getCategory()]++;
    }
    metrics.uniqueCategories = (int)uniqueCats.size();

    metrics.calculateDensity();
    metrics.calculateCategoryRatio();
    metrics.calculateCompositeScore();

    return metrics;
}


// ════════════════════════════════════════════════════════════════
//  RANK A LIST OF NAMED REGIONS
//
//  Analyzes each (Rectangle, name) pair, normalizes scores,
//  and prints the full ranked output.
// ════════════════════════════════════════════════════════════════
void rankRegions(RTree& tree,
                 const vector<pair<Rectangle, string>>& regionsToRank) {
    RegionRanker ranker;
    cout << "\n[System] Analyzing " << regionsToRank.size() << " regions...\n";
    for (const auto& [rect, name] : regionsToRank)
        ranker.addRegion(analyzeRegion(tree, rect, name));
    ranker.rankRegions();
    ranker.printRankings();
}


// ════════════════════════════════════════════════════════════════
//  AUTO GRID RANKING
//
//  Divides overallArea into (gridRows x gridCols) cells,
//  names each "Grid_R1C1" etc., then ranks them all.
// ════════════════════════════════════════════════════════════════
void rankGridRegions(RTree& tree,
                     const Rectangle& overallArea,
                     int gridRows,
                     int gridCols) {
    float minX  = overallArea.getMinX(), maxX = overallArea.getMaxX();
    float minY  = overallArea.getMinY(), maxY = overallArea.getMaxY();
    float stepX = (maxX - minX) / gridCols;
    float stepY = (maxY - minY) / gridRows;

    vector<pair<Rectangle, string>> regions;
    for (int row = 0; row < gridRows; row++) {
        for (int col = 0; col < gridCols; col++) {
            float x1 = minX + col * stepX;
            float y1 = minY + row * stepY;
            regions.push_back({
                Rectangle(x1, y1, x1 + stepX, y1 + stepY),
                "Grid_R" + to_string(row + 1) + "C" + to_string(col + 1)
            });
        }
    }
    rankRegions(tree, regions);
}


// ════════════════════════════════════════════════════════════════
//  QUADRANT SPLIT
//
//  Splits the entire dataset into four equal quadrants by:
//    1. Scanning all points to find the exact tight bounding box.
//    2. Computing midX and midY as the centre of that box.
//    3. Defining four rectangles (NW, NE, SW, SE).
//    4. Analyzing and ranking all four with the standard pipeline.
//
//  Visual layout:
//
//  maxY ┌──────────┬──────────┐
//       │  NW      │  NE      │
//  midY ├──────────┼──────────┤
//       │  SW      │  SE      │
//  minY └──────────┴──────────┘
//      minX      midX       maxX
// ════════════════════════════════════════════════════════════════
void rankTreeQuadrants(RTree& tree) {
    vector<Point> all = tree.getAllPoints();

    if (all.empty()) {
        cout << "[rankTreeQuadrants] Tree is empty — nothing to rank.\n";
        return;
    }

    // Step 1 — tight bounding box of all points
    float minX = all[0].getX(), maxX = all[0].getX();
    float minY = all[0].getY(), maxY = all[0].getY();
    for (const auto& p : all) {
        if (p.getX() < minX) minX = p.getX();
        if (p.getX() > maxX) maxX = p.getX();
        if (p.getY() < minY) minY = p.getY();
        if (p.getY() > maxY) maxY = p.getY();
    }

    // Step 2 — midpoints
    float midX = (minX + maxX) / 2.0f;
    float midY = (minY + maxY) / 2.0f;

    cout << "\n[rankTreeQuadrants] Whole-tree bounding box:\n";
    cout << "  X: " << minX << " to " << maxX << "  (midX = " << midX << ")\n";
    cout << "  Y: " << minY << " to " << maxY << "  (midY = " << midY << ")\n";

    // Step 3 — four quadrants
    vector<pair<Rectangle, string>> quadrants = {
        { Rectangle(minX, midY, midX, maxY), "NW Quadrant" },
        { Rectangle(midX, midY, maxX, maxY), "NE Quadrant" },
        { Rectangle(minX, minY, midX, midY), "SW Quadrant" },
        { Rectangle(midX, minY, maxX, midY), "SE Quadrant" }
    };

    // Step 4 — rank and print
    rankRegions(tree, quadrants);
}


// ════════════════════════════════════════════════════════════════
//  NEIGHBOURHOOD COMPARISON
//
//  Named alias for rankRegions with a decorative header.
// ════════════════════════════════════════════════════════════════
void compareNeighborhoods(RTree& tree,
                           const vector<pair<Rectangle, string>>& neighborhoods) {
    cout << "\n+=====================================================+\n";
    cout << "|        NEIGHBORHOOD COMPARISON ANALYSIS             |\n";
    cout << "+=====================================================+\n";
    rankRegions(tree, neighborhoods);
}

// ════════════════════════════════════════════════════════════════
//  FIND BEST REGION BY CRITERION
//
//  criterion options:
//    "density"    → highest points-per-unit-area wins
//    "ratio"      → highest categoryRatio wins
//    "categories" → most unique categories wins
//    "overall"    → normalized composite score (default)
// ════════════════════════════════════════════════════════════════
RegionMetrics findBestRegion(RTree& tree,
                              const vector<pair<Rectangle, string>>& regions,
                              const string& criterion = "overall") {
    RegionRanker ranker;
    for (const auto& [rect, name] : regions)
        ranker.addRegion(analyzeRegion(tree, rect, name));

    if (criterion == "density") {
        auto r = ranker.getRankings();
        sort(r.begin(), r.end(), [](const RegionMetrics& a, const RegionMetrics& b) {
            return a.density > b.density;
        });
        return r.empty() ? RegionMetrics() : r[0];
    }
    if (criterion == "ratio") {
        auto r = ranker.getRankings();
        sort(r.begin(), r.end(), [](const RegionMetrics& a, const RegionMetrics& b) {
            return a.categoryRatio > b.categoryRatio;
        });
        return r.empty() ? RegionMetrics() : r[0];
    }
    if (criterion == "categories") {
        auto r = ranker.getRankings();
        sort(r.begin(), r.end(), [](const RegionMetrics& a, const RegionMetrics& b) {
            return a.uniqueCategories > b.uniqueCategories;
        });
        return r.empty() ? RegionMetrics() : r[0];
    }

    // Default: overall composite score
    ranker.rankRegions();
    return ranker.getTopRegion();
}

#endif // REGION_RANKING_CPP