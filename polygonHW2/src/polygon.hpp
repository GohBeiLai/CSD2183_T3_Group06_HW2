#ifndef POLYGON_HPP
#define POLYGON_HPP

#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <map>
#include "ring.hpp"

class Polygon {
public:
    explicit Polygon(NodePool& pool) : pool_(pool) {}
    
    // Parse polygon from CSV file
    bool loadFromCSV(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            return false;
        }
        
        // Temporary storage for vertices grouped by ring_id
        struct VertexData {
            double x, y;
            int orig_id;
        };
        std::map<int, std::vector<VertexData>> ringVertices;
        
        std::string line;
        int lineNum = 0;
        while (std::getline(file, line)) {
            lineNum++;
            // Skip empty lines and header
            if (line.empty() || line[0] == 'r') continue;
            
            std::stringstream ss(line);
            std::string token;
            
            int ring_id, vertex_id;
            double x, y;
            
            try {
                std::getline(ss, token, ',');
                ring_id = std::stoi(token);
                
                std::getline(ss, token, ',');
                vertex_id = std::stoi(token);
                
                std::getline(ss, token, ',');
                x = std::stod(token);
                
                std::getline(ss, token, ',');
                y = std::stod(token);
            } catch (const std::exception&) {
                std::cerr << "Error parsing line " << lineNum << ": " << line << std::endl;
                continue;
            }
            
            ringVertices[ring_id].push_back({x, y, vertex_id});
        }
        
        // Create rings from parsed data
        for (const auto& [ring_id, vertices] : ringVertices) {
            auto ring = std::make_unique<Ring>(ring_id, pool_);
            for (const auto& v : vertices) {
                ring->addVertex(v.x, v.y, v.orig_id);
            }
            
            if (ring_id == 0) {
                exterior_ = std::move(ring);
            } else {
                interiors_.push_back(std::move(ring));
            }
        }
        
        return exterior_ != nullptr && exterior_->isValid();
    }
    
    // Get total vertex count across all rings (recalculated from linked lists)
    int totalVertexCount() const {
        int count = 0;
        if (exterior_ && exterior_->head()) {
            count = countVertices(exterior_->head());
        }
        for (const auto& ring : interiors_) {
            if (ring && ring->head()) {
                count += countVertices(ring->head());
            }
        }
        return count;
    }
    
    // Get exterior ring
    Ring* exterior() { return exterior_.get(); }
    const Ring* exterior() const { return exterior_.get(); }
    
    // Get interior rings
    std::vector<std::unique_ptr<Ring>>& interiors() { return interiors_; }
    const std::vector<std::unique_ptr<Ring>>& interiors() const { return interiors_; }
    
    // Get all rings (exterior + interiors)
    std::vector<Ring*> allRings() {
        std::vector<Ring*> rings;
        if (exterior_) rings.push_back(exterior_.get());
        for (auto& ring : interiors_) {
            rings.push_back(ring.get());
        }
        return rings;
    }
    
    // Calculate total signed area
    double totalSignedArea() const {
        double area = exterior_ ? exterior_->signedArea() : 0.0;
        for (const auto& ring : interiors_) {
            area += ring->signedArea();  // Interior rings have negative area (CW)
        }
        return area;
    }
    
    // Output polygon in required CSV format
    void outputCSV(std::ostream& out) const {
        out << "ring_id,vertex_id,x,y" << std::endl;
        
        int output_ring_id = 0;
        
        // Output exterior ring
        if (exterior_ && exterior_->head()) {
            outputRing(out, exterior_.get(), output_ring_id++);
        }
        
        // Output interior rings
        for (const auto& ring : interiors_) {
            if (ring && ring->head()) {
                outputRing(out, ring.get(), output_ring_id++);
            }
        }
    }
    
    // Check if polygon is valid (all rings have at least 3 vertices)
    bool isValid() const {
        if (!exterior_ || !exterior_->isValid()) return false;
        for (const auto& ring : interiors_) {
            if (!ring->isValid()) return false;
        }
        return true;
    }
    
    // Remove empty interior rings (rings with < 3 vertices)
    void removeInvalidInteriors() {
        interiors_.erase(
            std::remove_if(interiors_.begin(), interiors_.end(),
                          [](const std::unique_ptr<Ring>& r) { return !r->isValid(); }),
            interiors_.end()
        );
    }

private:
    void outputRing(std::ostream& out, const Ring* ring, int output_ring_id) const {
        if (!ring || !ring->head()) return;
        
        int vertex_id = 0;
        ring->forEachVertex([&](const Node* n) {
            out << output_ring_id << "," << vertex_id++ << "," 
                << n->x << "," << n->y << std::endl;
        });
    }
    
    NodePool& pool_;
    std::unique_ptr<Ring> exterior_;
    std::vector<std::unique_ptr<Ring>> interiors_;
};

#endif // POLYGON_HPP
