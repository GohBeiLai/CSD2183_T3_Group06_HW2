#ifndef SPATIAL_GRID_HPP
#define SPATIAL_GRID_HPP

#include <vector>
#include <unordered_set>
#include <cmath>
#include "geometry.hpp"
#include "node.hpp"

// Uniform grid spatial index for efficient edge intersection queries
class SpatialGrid {
public:
    struct Edge {
        Node* a;
        Node* b;
        int ring_id;
        
        bool operator==(const Edge& other) const {
            return (a == other.a && b == other.b) || (a == other.b && b == other.a);
        }
    };
    
    struct EdgeHash {
        size_t operator()(const Edge& e) const {
            // Order-independent hash
            auto h1 = std::hash<void*>{}(std::min(e.a, e.b));
            auto h2 = std::hash<void*>{}(std::max(e.a, e.b));
            return h1 ^ (h2 << 1);
        }
    };

    SpatialGrid() : cellSize_(1.0), gridWidth_(0), gridHeight_(0) {}
    
    // Build grid from polygon rings
    void build(const std::vector<Ring*>& rings, int cellCount = 100) {
        clear();
        
        if (rings.empty()) return;
        
        // Compute bounding box
        bbox_ = BBox();
        for (Ring* ring : rings) {
            ring->forEachVertex([this](const Node* n) {
                bbox_.expand(n);
            });
        }
        
        // Add small margin
        double margin = std::max(bbox_.width(), bbox_.height()) * 0.01;
        bbox_.minX -= margin;
        bbox_.minY -= margin;
        bbox_.maxX += margin;
        bbox_.maxY += margin;
        
        // Calculate cell size
        double maxDim = std::max(bbox_.width(), bbox_.height());
        cellSize_ = maxDim / cellCount;
        if (cellSize_ < EPSILON) cellSize_ = 1.0;
        
        gridWidth_ = static_cast<int>(std::ceil(bbox_.width() / cellSize_)) + 1;
        gridHeight_ = static_cast<int>(std::ceil(bbox_.height() / cellSize_)) + 1;
        
        // Resize grid
        cells_.resize(gridWidth_ * gridHeight_);
        
        // Insert all edges into grid
        for (Ring* ring : rings) {
            ring->forEachEdge([this, ring](Node* a, Node* b) {
                insertEdge(a, b, ring->ringId());
            });
        }
    }
    
    // Clear the grid
    void clear() {
        cells_.clear();
        gridWidth_ = 0;
        gridHeight_ = 0;
    }
    
    // Insert an edge into the grid
    void insertEdge(Node* a, Node* b, int ring_id) {
        auto cells = getCellsForSegment(a->x, a->y, b->x, b->y);
        Edge edge{a, b, ring_id};
        for (int cellIdx : cells) {
            if (cellIdx >= 0 && cellIdx < static_cast<int>(cells_.size())) {
                cells_[cellIdx].insert(edge);
            }
        }
    }
    
    // Remove an edge from the grid
    void removeEdge(Node* a, Node* b, int ring_id) {
        auto cells = getCellsForSegment(a->x, a->y, b->x, b->y);
        Edge edge{a, b, ring_id};
        for (int cellIdx : cells) {
            if (cellIdx >= 0 && cellIdx < static_cast<int>(cells_.size())) {
                cells_[cellIdx].erase(edge);
            }
        }
    }
    
    // Check if a segment intersects any other edge (not sharing vertices)
    bool segmentIntersectsAny(Node* p1, Node* p2, Node* exclude1 = nullptr, 
                              Node* exclude2 = nullptr, Node* exclude3 = nullptr,
                              Node* exclude4 = nullptr) const {
        auto cells = getCellsForSegment(p1->x, p1->y, p2->x, p2->y);
        Vec2 v1(p1), v2(p2);
        
        std::unordered_set<Edge, EdgeHash> checked;
        
        for (int cellIdx : cells) {
            if (cellIdx < 0 || cellIdx >= static_cast<int>(cells_.size())) continue;
            
            for (const Edge& edge : cells_[cellIdx]) {
                if (checked.count(edge)) continue;
                checked.insert(edge);
                
                // Skip edges that share a vertex with the test segment
                if (edge.a == p1 || edge.a == p2 || edge.b == p1 || edge.b == p2) continue;
                
                // Skip excluded nodes
                if (edge.a == exclude1 || edge.a == exclude2 || 
                    edge.a == exclude3 || edge.a == exclude4) continue;
                if (edge.b == exclude1 || edge.b == exclude2 || 
                    edge.b == exclude3 || edge.b == exclude4) continue;
                
                Vec2 e1(edge.a), e2(edge.b);
                if (segmentsIntersectProperly(v1, v2, e1, e2)) {
                    return true;
                }
            }
        }
        return false;
    }
    
    // Check if new edges after collapse would cause intersection.
    // Matches Project 2's collapse_causes_cross_ring_intersection logic:
    // - Only skip edges involving B or C (being removed), NOT A or D
    // - Use robust nontrivial intersection test (handles collinear overlaps)
    // - Check both same-ring and cross-ring intersections
    // - Check if E coincides with existing vertices
    bool collapseWouldIntersect(Node* a, Node* b, Node* c, Node* d,
                                const Vec2& newE, int collapse_ring_id) const {
        Vec2 vA(a), vD(d);

        // Degenerate: E coincides with A or D
        if (point_eq(vA, newE) || point_eq(vD, newE)) return true;

        // Phase 1: Check if E coincides with any existing vertex (via grid point query)
        int eCell = getCellIndex(newE.x, newE.y);
        if (eCell >= 0 && eCell < static_cast<int>(cells_.size())) {
            for (const Edge& edge : cells_[eCell]) {
                Vec2 ea(edge.a), eb(edge.b);
                if (edge.ring_id == collapse_ring_id) {
                    if (edge.a != a && edge.a != b && edge.a != c && edge.a != d && point_eq(ea, newE)) return true;
                    if (edge.b != a && edge.b != b && edge.b != c && edge.b != d && point_eq(eb, newE)) return true;
                } else {
                    if (point_eq(ea, newE) || point_eq(eb, newE)) return true;
                }
            }
        }

        // Phase 2: Check edge intersections
        auto cellsAE = getCellsForSegment(a->x, a->y, newE.x, newE.y);
        auto cellsED = getCellsForSegment(newE.x, newE.y, d->x, d->y);

        std::unordered_set<int> allCells;
        for (int ci : cellsAE) allCells.insert(ci);
        for (int ci : cellsED) allCells.insert(ci);

        std::unordered_set<Edge, EdgeHash> checked;

        for (int cellIdx : allCells) {
            if (cellIdx < 0 || cellIdx >= static_cast<int>(cells_.size())) continue;

            for (const Edge& edge : cells_[cellIdx]) {
                if (checked.count(edge)) continue;
                checked.insert(edge);

                Vec2 ea(edge.a), eb(edge.b);

                if (edge.ring_id == collapse_ring_id) {
                    // Same ring: only skip edges involving B or C (being removed)
                    if (edge.a == b || edge.a == c || edge.b == b || edge.b == c) continue;

                    // Nontrivial intersection with shared endpoint handling
                    if (segmentsIntersectNontrivial(vA, newE, ea, eb, true)) return true;
                    if (segmentsIntersectNontrivial(newE, vD, ea, eb, true)) return true;
                } else {
                    // Cross ring: check all edges, no shared endpoint exemption
                    if (segmentsIntersectNontrivial(vA, newE, ea, eb, false)) return true;
                    if (segmentsIntersectNontrivial(newE, vD, ea, eb, false)) return true;
                }
            }
        }

        return false;
    }
    
    // Update grid after a collapse operation
    void updateAfterCollapse(Node* a, Node* b, Node* c, Node* d, 
                            Node* newE, int ring_id) {
        // Remove old edges: A-B, B-C, C-D
        removeEdge(a, b, ring_id);
        removeEdge(b, c, ring_id);
        removeEdge(c, d, ring_id);
        
        // Add new edges: A-E, E-D
        insertEdge(a, newE, ring_id);
        insertEdge(newE, d, ring_id);
    }

private:
    // Get cell index for a point
    int getCellIndex(double x, double y) const {
        if (gridWidth_ == 0 || gridHeight_ == 0) return -1;
        
        int cx = static_cast<int>((x - bbox_.minX) / cellSize_);
        int cy = static_cast<int>((y - bbox_.minY) / cellSize_);
        
        cx = std::max(0, std::min(cx, gridWidth_ - 1));
        cy = std::max(0, std::min(cy, gridHeight_ - 1));
        
        return cy * gridWidth_ + cx;
    }
    
    // Get all cells that a segment passes through (using DDA-like algorithm)
    std::vector<int> getCellsForSegment(double x1, double y1, double x2, double y2) const {
        std::vector<int> result;
        if (gridWidth_ == 0 || gridHeight_ == 0) return result;
        
        // Get cell coordinates
        int cx1 = static_cast<int>((x1 - bbox_.minX) / cellSize_);
        int cy1 = static_cast<int>((y1 - bbox_.minY) / cellSize_);
        int cx2 = static_cast<int>((x2 - bbox_.minX) / cellSize_);
        int cy2 = static_cast<int>((y2 - bbox_.minY) / cellSize_);
        
        // Clamp to grid bounds
        cx1 = std::max(0, std::min(cx1, gridWidth_ - 1));
        cy1 = std::max(0, std::min(cy1, gridHeight_ - 1));
        cx2 = std::max(0, std::min(cx2, gridWidth_ - 1));
        cy2 = std::max(0, std::min(cy2, gridHeight_ - 1));
        
        // Simple approach: include all cells in bounding box
        int minCx = std::min(cx1, cx2);
        int maxCx = std::max(cx1, cx2);
        int minCy = std::min(cy1, cy2);
        int maxCy = std::max(cy1, cy2);
        
        for (int cy = minCy; cy <= maxCy; cy++) {
            for (int cx = minCx; cx <= maxCx; cx++) {
                result.push_back(cy * gridWidth_ + cx);
            }
        }
        
        return result;
    }
    
    BBox bbox_;
    double cellSize_;
    int gridWidth_, gridHeight_;
    std::vector<std::unordered_set<Edge, EdgeHash>> cells_;
};

#endif // SPATIAL_GRID_HPP
