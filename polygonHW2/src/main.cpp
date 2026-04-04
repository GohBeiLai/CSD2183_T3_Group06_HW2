#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

#include "node.hpp"
#include "polygon.hpp"
#include "apsc.hpp"
#include "symmetric_difference.hpp"

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " <input.csv> <target_vertices>" << std::endl;
    std::cerr << "  input.csv      - Input polygon CSV file" << std::endl;
    std::cerr << "  target_vertices - Target number of vertices (integer)" << std::endl;
}

// Read original vertices from CSV, grouped by ring_id
std::map<int, std::vector<std::pair<double, double>>> readOriginalVertices(const std::string& filename) {
    std::map<int, std::vector<std::pair<double, double>>> rings;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == 'r') continue;
        std::stringstream ss(line);
        std::string tok;
        std::getline(ss, tok, ','); int ring_id = std::stoi(tok);
        std::getline(ss, tok, ','); // vertex_id
        std::getline(ss, tok, ','); double x = std::stod(tok);
        std::getline(ss, tok, ','); double y = std::stod(tok);
        rings[ring_id].push_back({ x, y });
    }
    return rings;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }

    std::string inputFile = argv[1];
    int targetVertices;

    try {
        targetVertices = std::stoi(argv[2]);
        if (targetVertices < 3) {
            std::cerr << "Error: Target vertices must be at least 3" << std::endl;
            return 1;
        }
    }
    catch (const std::exception&) {
        std::cerr << "Error: Invalid target vertices: " << argv[2] << std::endl;
        return 1;
    }

    // Store original vertices for symmetric difference computation
    auto originalRings = readOriginalVertices(inputFile);

    // Create node pool and polygon
    NodePool pool;
    Polygon polygon(pool);

    // Load polygon from CSV
    if (!polygon.loadFromCSV(inputFile)) {
        std::cerr << "Error: Failed to load polygon from " << inputFile << std::endl;
        return 1;
    }

    // Record input area
    double inputArea = polygon.totalSignedArea();
    std::cerr << "Loaded polygon with " << polygon.totalVertexCount() << " vertices" << std::endl;

    // Run APSC simplification
    APSC apsc(polygon);
    int finalVertices = apsc.simplify(targetVertices);

    // Record output area
    double outputArea = polygon.totalSignedArea();

    // Get simplified vertices grouped by ring for symmetric difference
    std::map<int, std::vector<std::pair<double, double>>> simplifiedRings;
    int output_ring_id = 0;
    if (polygon.exterior() && polygon.exterior()->head()) {
        simplifiedRings[output_ring_id] = polygon.exterior()->getVertices();
        output_ring_id++;
    }
    for (auto& ring : polygon.interiors()) {
        if (ring && ring->head()) {
            simplifiedRings[output_ring_id] = ring->getVertices();
            output_ring_id++;
        }
    }

    // Compute total areal displacement as the geometric symmetric difference
    // between original and simplified polygons, computed per ring and summed
    double totalDisplacement = 0.0;
    for (auto& [rid, origVerts] : originalRings) {
        if (simplifiedRings.count(rid) && origVerts.size() >= 3 && simplifiedRings[rid].size() >= 3) {
            totalDisplacement += symdiff::computeSymmetricDifference(origVerts, simplifiedRings[rid]);
        }
    }

    // Set format for coordinates
    std::cout << std::setprecision(10);

    // Output simplified polygon to stdout
    polygon.outputCSV(std::cout);

    // Output statistics in required format
    std::cout << std::scientific << std::setprecision(6);
    std::cout << "Total signed area in input: " << inputArea << std::endl;
    std::cout << "Total signed area in output: " << outputArea << std::endl;
    std::cout << "Total areal displacement: " << totalDisplacement << std::endl;

    std::cerr << "Simplification complete: " << finalVertices << " vertices" << std::endl;

    return 0;
}
