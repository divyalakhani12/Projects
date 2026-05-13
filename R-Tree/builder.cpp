#ifndef BUILDER_CPP
#define BUILDER_CPP

#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <fstream>

#include "RTree.cpp"

using namespace std;

string trim(string s) {
    while (!s.empty() && isspace(s.front())) {
        s.erase(s.begin());
    }
    while (!s.empty() && isspace(s.back())) {
        s.pop_back();
    }
    return s;
}


// ===================== CSV FUNCTIONS =====================
vector<string> splitCSVLine(const string& line) {
    vector<string> result;
    string cell;
    bool inQuotes = false;

    for (char ch : line) {
        if (ch == '"') {
            inQuotes = !inQuotes;
        } else if (ch == ',' && !inQuotes) {
            result.push_back(cell);
            cell.clear();
        } else {
            cell += ch;
        }
    }

    result.push_back(cell);
    return result;
}

void loadCSVIntoTree(const string& csvFile, RTree& tree) {
    ifstream file(csvFile);

    if (!file.is_open()) {
        cout << "Could not open CSV file.\n";
        return;
    }

    string line;
    getline(file, line); // skip header

    int inserted = 0;
    int skipped = 0;

    while (getline(file, line)) {
    if (line.empty()) {
        skipped++;
        cout << "Skipped: empty line\n";
        continue;
    }

    vector<string> row = splitCSVLine(line);

    if (row.size() < 8) {
        skipped++;
        cout << "Skipped: short row -> " << line << endl;
        continue;
    }

    try {
        string name = trim(row[0]);
        float x = stof(row[5]);
        float y = stof(row[6]);
        string category = trim(row[7]);

        Point p(x, y, name, category);
        tree.insert(p);
        inserted++;
    }
    catch (const exception& e) {
        skipped++;
        cout << "Skipped: bad numeric conversion -> " << line << endl;
        cout << "x field: " << row[5] << ", y field: " << row[6] << endl;
    }
    }

    file.close();

    cout << "Inserted rows: " << inserted << endl;
    cout << "Skipped rows: " << skipped << endl;
}

#endif // BUILDER_CPP
