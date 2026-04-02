#ifndef APSC_HPP
#define APSC_HPP

#include <iostream>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include "polygon.hpp"
#include "spatial_grid.hpp"
#include "priority_queue.hpp"

// Area-Preserving Segment Collapse algorithm
class APSC {
public:
    APSC(Polygon& polygon) : polygon_(polygon), totalDisplacement_(0.0) {}
    
    // Run the simplification algorithm
    // Returns the actual number of vertices after simplification
    int simplify(int targetVertices) {
        int currentVertices = polygon_.totalVertexCount();
        
        if (currentVertices <= targetVertices) {
            std::cerr << "Already at or below target vertex count" << std::endl;
            return currentVertices;
        }
        
        // Build spatial index
        auto rings = polygon_.allRings();
        grid_.build(rings);
        
        // Initialize ring vertex counts
        for (Ring* ring : rings) {
            ringVertexCounts_[ring->ringId()] = countVertices(ring->head());
        }
        
        // Initialize priority queue with all possible collapses
        initializeCollapses();
        
        std::cerr << "Starting APSC with " << currentVertices << " vertices, target: " 
                  << targetVertices << std::endl;
        std::cerr << "Initial collapse candidates: " << pq_.size() << std::endl;
        
        int iterations = 0;
        while (currentVertices > targetVertices) {
            CollapseCandidate best;
            
            // Find valid collapse with minimum displacement
            if (!pq_.pop(best)) {
                std::cerr << "No more valid collapses available" << std::endl;
                break;
            }
            
            // Verify the collapse is still valid after popping
            if (!best.isValid()) {
                continue;  // Skip stale entry
            }
            
            // Find the ring for this collapse
            Ring* ring = findRing(best.ring_id);
            if (!ring) {
                std::cerr << "Ring not found: " << best.ring_id << std::endl;
                continue;
            }
            
            // Check ring vertex count from our cached map
            int ringVertices = ringVertexCounts_[best.ring_id];
            
            // Check minimum vertex constraint (ring must have at least 3 vertices after collapse)
            // We remove 1 vertex per collapse, so need at least 4 to end up with 3
            if (ringVertices <= 3) {
                continue;
            }
            
            // Topology check: verify collapse won't cause intersections
            // The spatial grid checks against all edges in all rings
            if (grid_.collapseWouldIntersect(best.a, best.b, best.c, best.d, best.newE)) {
                continue;
            }
            
            // Apply the collapse
            applyCollapse(ring, best);
            
            // The collapse A-B-C-D -> A-E-D removes one vertex (C), 
            // and repurposes B as E. Net reduction: 1 vertex
            currentVertices -= 1;
            ringVertexCounts_[best.ring_id] -= 1;
            
            totalDisplacement_ += best.displacement;
            iterations++;
            
            // Add new collapse candidates for affected vertices
            Node* eNode = best.b;  // B is now E
            addNewCandidates(ring, best.a, eNode, best.d);
            
            // Periodic compaction of priority queue
            if (iterations % 100 == 0) {
                pq_.compact();
                std::cerr << "Iteration " << iterations << ", vertices: " << currentVertices << std::endl;
            }
        }
        
        // Clean up any invalid interior rings
        polygon_.removeInvalidInteriors();
        
        std::cerr << "Finished after " << iterations << " iterations" << std::endl;
        std::cerr << "Final vertex count: " << polygon_.totalVertexCount() << std::endl;
        
        return polygon_.totalVertexCount();
    }
    
    // Get total areal displacement
    double totalDisplacement() const { return totalDisplacement_; }

private:
    // Find ring by ID
    Ring* findRing(int ring_id) {
        if (ring_id == 0) {
            return polygon_.exterior();
        }
        for (auto& ring : polygon_.interiors()) {
            if (ring->ringId() == ring_id) {
                return ring.get();
            }
        }
        return nullptr;
    }
    
    // Initialize priority queue with all possible collapses
    void initializeCollapses() {
        for (Ring* ring : polygon_.allRings()) {
            int vertCount = ringVertexCounts_[ring->ringId()];
            if (vertCount < 4) continue;  // Need at least 4 vertices to have a valid collapse
            
            ring->forEachVertex([this, ring, vertCount](Node* node) {
                // Create collapse candidate for A=node, B=next, C=next->next, D=next->next->next
                Node* a = node;
                Node* b = a->next;
                Node* c = b->next;
                Node* d = c->next;
                
                // For rings, the list wraps around, check we don't have duplicate nodes
                if (d == a || c == a || b == a) return;
                if (vertCount < 4) return;
                
                CollapseCandidate candidate(a, b, c, d, ring->ringId());
                pq_.push(candidate);
            });
        }
    }
    
    // Apply a collapse operation
    // A -> B -> C -> D becomes A -> E -> D (where E replaces B's position with new coordinates)
    void applyCollapse(Ring* ring, const CollapseCandidate& collapse) {
        Node* a = collapse.a;
        Node* b = collapse.b;
        Node* c = collapse.c;
        Node* d = collapse.d;
        
        // Update spatial grid: remove old edges A-B, B-C, C-D
        grid_.removeEdge(a, b, ring->ringId());
        grid_.removeEdge(b, c, ring->ringId());
        grid_.removeEdge(c, d, ring->ringId());
        
        // Update B's position to E (reuse node B as the new point E)
        b->x = collapse.newE.x;
        b->y = collapse.newE.y;
        b->generation++;  // Invalidate any old references to B
        
        // Remove C from the ring by relinking: B->next = D, D->prev = B
        b->next = d;
        d->prev = b;
        
        // If C was the head of the ring, update head
        if (ring->head() == c) {
            ring->setHead(d);
        }
        
        // Mark C as invalid
        c->generation++;
        c->prev = nullptr;
        c->next = nullptr;
        
        // Update spatial grid: add new edges A-E, E-D
        grid_.insertEdge(a, b, ring->ringId());  // A-E (B is now E)
        grid_.insertEdge(b, d, ring->ringId());  // E-D
    }
    
    // Add new collapse candidates after a collapse
    // After A-B-C-D -> A-E-D, we need new candidates involving A, E(=B), D
    void addNewCandidates(Ring* ring, Node* a, Node* e, Node* d) {
        int vertCount = ringVertexCounts_[ring->ringId()];
        if (vertCount < 4) return;
        
        // Generate candidates in a wider neighborhood
        // Candidate 1: prev(A) - A - E - D
        Node* prevA = a->prev;
        if (prevA && prevA != e && prevA != d) {
            pq_.push(CollapseCandidate(prevA, a, e, d, ring->ringId()));
        }
        
        // Candidate 2: A - E - D - next(D)
        Node* nextD = d->next;
        if (nextD && nextD != a && nextD != e) {
            pq_.push(CollapseCandidate(a, e, d, nextD, ring->ringId()));
        }
        
        // Candidate 3: prev(prev(A)) - prev(A) - A - E
        if (prevA && prevA->prev && prevA->prev != d && prevA->prev != e) {
            pq_.push(CollapseCandidate(prevA->prev, prevA, a, e, ring->ringId()));
        }
        
        // Candidate 4: E - D - next(D) - next(next(D))
        if (nextD && nextD->next && nextD->next != a && nextD->next != e) {
            pq_.push(CollapseCandidate(e, d, nextD, nextD->next, ring->ringId()));
        }
    }
    
    // Check if collapse would cause self-intersection within the ring
    // Uses spatial grid for efficient intersection queries
    bool collapseWouldSelfIntersect(Ring* /*ring*/, Node* a, Node* b, Node* c, Node* d, 
                                     const Vec2& newE) const {
        // Use spatial grid to check for intersections
        // The grid already knows about all edges in the polygon
        return grid_.collapseWouldIntersect(a, b, c, d, newE);
    }
    
    Polygon& polygon_;
    SpatialGrid grid_;
    CollapsePriorityQueue pq_;
    double totalDisplacement_;
    std::unordered_map<int, int> ringVertexCounts_;
};

#endif // APSC_HPP
