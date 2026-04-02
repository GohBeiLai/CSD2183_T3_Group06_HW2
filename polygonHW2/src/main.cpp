#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>

#include "node.hpp"
#include "polygon.hpp"
#include "apsc.hpp"

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " <input.csv> <target_vertices>" << std::endl;
    std::cerr << "  input.csv      - Input polygon CSV file" << std::endl;
    std::cerr << "  target_vertices - Target number of vertices (integer)" << std::endl;
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
    } catch (const std::exception&) {
        std::cerr << "Error: Invalid target vertices: " << argv[2] << std::endl;
        return 1;
    }
    
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
    double displacement = apsc.totalDisplacement();
    
    // Set format for coordinates: use high precision but avoid scientific if possible
    std::cout << std::setprecision(10);
    
    // Output simplified polygon to stdout
    polygon.outputCSV(std::cout);
    
    // Output statistics exactly in the required format
    std::cout << std::scientific << std::setprecision(6);
    std::cout << "Total signed area in input: " << inputArea << std::endl;
    std::cout << "Total signed area in output: " << outputArea << std::endl;
    std::cout << "Total areal displacement: " << displacement << std::endl;
    
    std::cerr << "Simplification complete: " << finalVertices << " vertices" << std::endl;
    
    return 0;
}
