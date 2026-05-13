#include <iostream>
#include <vector>
#include <string>
#include "builder.cpp"
#include "region_ranking.cpp"
#include "AreaCoverage.cpp"

using namespace std;


// ════════════════════════════════════════════════════════════════
//  UTILITY
// ════════════════════════════════════════════════════════════════

void printHeader(const string& title) {
    cout << "\n========================================\n";
    cout << "  DEMO: " << title << "\n";
    cout << "========================================\n";
}


// ════════════════════════════════════════════════════════════════
//  MAIN
// ════════════════════════════════════════════════════════════════

int main() {
    RTree tree;
    const string csvFile = "cleaned_locations.csv";
    const string datFile = "rtree_data.dat";


    // ── 1. DATA LOADING & PERSISTENCE ───────────────────────────
    printHeader("Data Loading & Persistence");

    if (tree.treeFileExists(datFile)) {
        cout << "[System] Loading optimized R-Tree from " << datFile << "...\n";
        tree.loadTreeFromFile(datFile);
    } else {
        cout << "[System] Building R-Tree from CSV...\n";
        loadCSVIntoTree(csvFile, tree);
        tree.saveTreeToFile(datFile);
        cout << "[System] Tree saved for future use.\n";
    }


    // ── 2. RANGE SEARCH ─────────────────────────────────────────
    printHeader("Range Search (Rectangular Query)");

    Rectangle areaQuery(24.80f, 66.98f, 24.95f, 67.15f);
    vector<Point> areaResults = tree.search(areaQuery);
    cout << "Found " << areaResults.size()
         << " locations in the specified rectangle.\n";
    if (!areaResults.empty())
        cout << "First match: " << areaResults[0].getName() << "\n";


    // ── 3. NEAREST NEIGHBOR ──────────────────────────────────────
    printHeader("Nearest Neighbor Search");

    float testLat = 24.8607f, testLon = 67.0011f;
    Point nearest = tree.nearestNeighbor(testLat, testLon);
    cout << "Closest to (" << testLat << ", " << testLon << "): "
         << nearest.getName() << " [" << nearest.getCategory() << "]\n";


    // ── 4. K-NEAREST NEIGHBORS ───────────────────────────────────
    printHeader("K-Nearest Neighbors (K=3)");

    int K = 3;
    vector<Point> knnResults = tree.k_nearestNeighbors(testLat, testLon, K);
    for (int i = 0; i < (int)knnResults.size(); i++)
        cout << i + 1 << ". " << knnResults[i].getName() << "\n";


    // ── 5. NAME & CATEGORY SEARCH ────────────────────────────────
    printHeader("Name & Category Search");

    string searchName = "Aqsa";
    vector<Point> nameMatches = tree.searchByName(searchName);
    cout << "Searching for '" << searchName << "': Found "
         << nameMatches.size() << " matches.\n";

    vector<string> cats = {"Restaurant", "Healthcare"};
    vector<Point> catMatches = tree.searchByCategory(cats);
    cout << "Category filter {Restaurant, Healthcare}: Found "
         << catMatches.size() << " matches.\n";


    // ── 6. COMBINED QUERIES ──────────────────────────────────────
    printHeader("Combined Query: Name + Category");

    Query nameHealthcareQuery;
    nameHealthcareQuery.name       = "Aqsa";
    nameHealthcareQuery.categories = {"Healthcare"};

    vector<Point> nameHealthcareMatches = tree.executeQuery(nameHealthcareQuery);
    cout << "Searching for 'Aqsa' in Healthcare: Found "
         << nameHealthcareMatches.size() << " matches.\n";
    for (int i = 0; i < (int)nameHealthcareMatches.size(); i++)
        cout << i + 1 << ". " << nameHealthcareMatches[i].getName()
             << " [" << nameHealthcareMatches[i].getCategory() << "]\n";

    // ── Range + Category ─────────────────────────────────────────
    printHeader("Combined Query: Range + Category");

    Query rangeHealthcareQuery;
    rangeHealthcareQuery.useRange   = true;
    rangeHealthcareQuery.range      = areaQuery;
    rangeHealthcareQuery.categories = {"Healthcare"};

    vector<Point> areaHealthcareResults = tree.executeQuery(rangeHealthcareQuery);
    cout << "Healthcare locations in rectangle: "
         << areaHealthcareResults.size() << "\n";
    if (!areaHealthcareResults.empty())
        cout << "First match: " << areaHealthcareResults[0].getName()
             << " [" << areaHealthcareResults[0].getCategory() << "]\n";

    // ── Nearest + Category ───────────────────────────────────────
    printHeader("Combined Query: Nearest + Category");

    Query nearestHealthcareQuery;
    nearestHealthcareQuery.useNearest  = true;
    nearestHealthcareQuery.x           = testLat;
    nearestHealthcareQuery.y           = testLon;
    nearestHealthcareQuery.categories  = {"Healthcare"};

    vector<Point> nearestHealthcare = tree.executeQuery(nearestHealthcareQuery);
    if (!nearestHealthcare.empty())
        cout << "Nearest Healthcare to (" << testLat << ", " << testLon << "): "
             << nearestHealthcare[0].getName()
             << " [" << nearestHealthcare[0].getCategory() << "]\n";

    // ── K-Nearest + Category ─────────────────────────────────────
    printHeader("Combined Query: K-Nearest + Category");

    Query kNearestHealthcareQuery;
    kNearestHealthcareQuery.useKNearest = true;
    kNearestHealthcareQuery.k           = 3;
    kNearestHealthcareQuery.x           = testLat;
    kNearestHealthcareQuery.y           = testLon;
    kNearestHealthcareQuery.categories  = {"Healthcare"};

    vector<Point> healthcareNeighbors = tree.executeQuery(kNearestHealthcareQuery);
    cout << "3 nearest Healthcare locations:\n";
    for (int i = 0; i < (int)healthcareNeighbors.size(); i++)
        cout << i + 1 << ". " << healthcareNeighbors[i].getName()
             << " [" << healthcareNeighbors[i].getCategory() << "]\n";


    // ── 7. PLACEMENT SUGGESTION ──────────────────────────────────
    printHeader("Best Placement Suggestion");

    string targetCat = "Healthcare";
    cout << "Finding the best location for a new " << targetCat << "...\n";
    tree.bestPlacementSuggestion(targetCat);


    // ── 8. UPDATE OPERATION ──────────────────────────────────────
    printHeader("Update Operation");

    string oldName = "BMI Physio";
    string newName = "BMI Physio Updated";

    vector<Point> beforeUpdate = tree.searchByName(oldName);
    cout << "Matches for '" << oldName << "' before update: "
         << beforeUpdate.size() << "\n";

    if (!beforeUpdate.empty()) {
        Point updatedPoint(beforeUpdate[0].getX(), beforeUpdate[0].getY(),
                           newName, "Healthcare");
        bool updated = tree.updateByName(oldName, updatedPoint);
        cout << "Update status: " << (updated ? "success" : "failed") << "\n";

        Point exactOldMatch, exactNewMatch;
        cout << "Old name still exists: "
             << (tree.findPlaceByName(oldName, exactOldMatch) ? "yes" : "no") << "\n";
        cout << "New name now exists:   "
             << (tree.findPlaceByName(newName, exactNewMatch) ? "yes" : "no") << "\n";
    }


    // ── 9. NEIGHBOURHOOD COMPARISON ─────────────────────────────
    printHeader("Region Ranking - Neighborhood Comparison");

    vector<pair<Rectangle, string>> neighborhoods = {
        { Rectangle(24.80f, 66.98f, 24.85f, 67.05f), "Saddar Area"  },
        { Rectangle(24.85f, 67.00f, 24.90f, 67.08f), "Clifton Area" },
        { Rectangle(24.90f, 67.05f, 24.95f, 67.12f), "DHA Area"     }
    };
    compareNeighborhoods(tree, neighborhoods);


    // ── 10. GRID ANALYSIS ────────────────────────────────────────
    printHeader("Region Ranking - Grid-Based Analysis (2x2)");

    Rectangle overallArea(24.80f, 66.98f, 24.95f, 67.15f);
    rankGridRegions(tree, overallArea, 2, 2);


    // ── 11. QUADRANT SPLIT ───────────────────────────────────────
    printHeader("Region Ranking - Whole-Tree Quadrant Split");

    rankTreeQuadrants(tree);


    // ── 12. BEST REGION BY CRITERION ────────────────────────────
    printHeader("Region Ranking - Best Region by Criterion");

    vector<pair<Rectangle, string>> zones = {
        { Rectangle(24.82f, 67.00f, 24.87f, 67.05f), "Zone A" },
        { Rectangle(24.87f, 67.03f, 24.92f, 67.08f), "Zone B" },
        { Rectangle(24.88f, 67.06f, 24.93f, 67.11f), "Zone C" }
    };

    cout << "\nFinding zone with HIGHEST DENSITY:\n";
    RegionMetrics densest = findBestRegion(tree, zones, "density");
    if (densest.pointCount > 0)
        cout << "  Winner: " << densest.regionName
             << " (density: " << densest.density << " pts/unit²)\n";

    cout << "\nFinding zone with BEST CATEGORY RATIO:\n";
    RegionMetrics diverse = findBestRegion(tree, zones, "ratio");
    if (diverse.pointCount > 0)
        cout << "  Winner: " << diverse.regionName
             << " (category ratio: " << diverse.categoryRatio << ")\n";

    cout << "\nFinding zone with MOST UNIQUE CATEGORIES:\n";
    RegionMetrics mostCat = findBestRegion(tree, zones, "categories");
    if (mostCat.pointCount > 0)
        cout << "  Winner: " << mostCat.regionName
             << " (" << mostCat.uniqueCategories << " unique categories)\n";

    cout << "\nFinding zone with BEST OVERALL SCORE:\n";
    RegionMetrics overall = findBestRegion(tree, zones, "overall");
    if (overall.pointCount > 0)
        cout << "  Winner: " << overall.regionName
             << " (score: " << overall.totalScore << ")\n";


    // ── 13. DETAILED SINGLE REGION ANALYSIS ─────────────────────
    printHeader("Region Ranking - Detailed Single Region");

    Rectangle targetZone(24.85f, 67.02f, 24.92f, 67.09f);
    RegionMetrics detailed = analyzeRegion(tree, targetZone, "Target Development Zone");
    detailed.print();

    // ── 14. AREA COVERAGE ANALYSIS ─────────────────────
    printHeader("Area Coverage Analysis");
    CoverageConfig cfg;
    cfg.addCategory("Healthcare", 0.05);
    cfg.addCategory("Restaurant", 0.05);

    Rectangle region(24.80, 66.98, 24.95, 67.15);
    CoverageAnalyzer analyzer(tree);

    CoverageResult result = analyzer.analyze(region, cfg, 10, 10);
    result.printSummary(cfg);
    analyzer.printCoverageAt(testLat, testLon, cfg);

    tree.saveTreeToFile(datFile);
    cout << "\n[Demo Complete]" << endl;


    // ── PERSIST & SUMMARISE ──────────────────────────────────────
    tree.saveTreeToFile(datFile);

    cout << "\n========================================\n";
    cout << "  Feature Summary\n";
    cout << "========================================\n";
    cout << "  Range Search\n";
    cout << "  Nearest Neighbor (NN & K-NN)\n";
    cout << "  Name & Category Filtering\n";
    cout << "  Combined Queries\n";
    cout << "  Best Placement Suggestion\n";
    cout << "  Update Operation\n";
    cout << "  Region Ranking\n";
    cout << "    - Neighborhood comparison\n";
    cout << "    - Grid-based analysis\n";
    cout << "    - Quadrant split\n";
    cout << "    - Best region by criterion\n";
    cout << "    - Detailed single-region report\n";
    cout << "========================================\n";

    cout << "\n[Demo Complete]\n";
    return 0;
}