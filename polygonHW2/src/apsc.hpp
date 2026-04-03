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
            
            // Topology check: O(n) scan matching Project 2's exact logic
            if (collapseCausesIntersection(ring, best.a, best.b, best.c, best.d, best.newE)) {
                continue;
            }
            
            // Apply the collapse
            applyCollapse(ring, best);
            
            // The collapse A-B-C-D -> A-E-D removes B and C (-2), adds E (+1)
            // Net reduction: 1 vertex
            currentVertices -= 1;
            ringVertexCounts_[best.ring_id] -= 1;
            
            totalDisplacement_ += best.displacement;
            iterations++;
            
            // Add new collapse candidates for affected vertices
            Node* eNode = lastInsertedE_;
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
    // A -> B -> C -> D becomes A -> E -> D
    // Following Project 2's approach: remove B and C, insert new vertex E after A
    void applyCollapse(Ring* ring, const CollapseCandidate& collapse) {
        Node* a = collapse.a;
        Node* b = collapse.b;
        Node* c = collapse.c;
        Node* d = collapse.d;
        
        // Update spatial grid: remove old edges A-B, B-C, C-D
        grid_.removeEdge(a, b, ring->ringId());
        grid_.removeEdge(b, c, ring->ringId());
        grid_.removeEdge(c, d, ring->ringId());
        
        // Remove B from the ring (this also increments generation and decrements count)
        ring->removeNode(b);
        
        // Remove C from the ring
        ring->removeNode(c);
        
        // Insert new vertex E after A (increments count)
        Node* e = ring->insertAfter(a, collapse.newE.x, collapse.newE.y);
        
        // Update spatial grid: add new edges A-E, E-D
        grid_.insertEdge(a, e, ring->ringId());
        grid_.insertEdge(e, d, ring->ringId());
        
        // Store E for generating new candidates
        lastInsertedE_ = e;
    }
    
    // Track the last inserted E vertex for generating new candidates
    Node* lastInsertedE_ = nullptr;
    
    // Add new collapse candidates after a collapse
    // After A-B-C-D -> A-E-D, we need to regenerate candidates in the affected neighborhood
    // Following the same pattern as the reference implementation:
    // Generate 4 candidates starting from E->prev->prev->prev
    void addNewCandidates(Ring* ring, Node* /*a*/, Node* e, Node* /*d*/) {
        int vertCount = ringVertexCounts_[ring->ringId()];
        if (vertCount < 4) return;
        
        // Start from E->prev->prev->prev and generate 4 consecutive candidates
        Node* start = e->prev->prev->prev;
        for (int i = 0; i < 4; ++i) {
            Node* A = start;
            Node* B = A->next;
            Node* C = B->next;
            Node* D = C->next;
            pq_.push(CollapseCandidate(A, B, C, D, ring->ringId()));
            start = start->next;
        }
    }
    
    // Check if collapse would cause self-intersection within the ring
    // Uses spatial grid for efficient intersection queries
    bool collapseWouldSelfIntersect(Ring* /*ring*/, Node* a, Node* b, Node* c, Node* d, 
                                     const Vec2& newE) const {
        // Use spatial grid to check for intersections
        // The grid already knows about all edges in the polygon
        return grid_.collapseWouldIntersect(a, b, c, d, newE, b->ring_id);
    }
    
    // Exact port of Project 2's collapse_causes_intersection + collapse_causes_cross_ring_intersection.
    // Uses O(n) scan (no spatial grid) for correctness parity.
    bool collapseCausesIntersection(Ring* ring, Node* A, Node* B, Node* C, Node* D, const Vec2& E) {
        Vec2 vA(A), vD(D);

        // E coincides with A or D → degenerate
        if (point_eq(vA, E) || point_eq(vD, E)) return true;

        // Check if E coincides with any vertex in the same ring (excluding ONLY A and D)
        // Project 2 rejects collapses where E coincides with B or C too
        {
            Node* v = ring->head();
            do {
                if (v != A && v != D && point_eq(Vec2(v), E)) return true;
                v = v->next;
            } while (v != ring->head());
        }

        // Same-ring edge check: skip edges involving B or C
        {
            Node* u = ring->head();
            do {
                Node* w = u->next;
                if (u == B || u == C || w == B || w == C) {
                    u = w;
                    continue;
                }
                Vec2 pu(u), pw(w);
                if (segmentsIntersectNontrivial(vA, E, pu, pw, true)) return true;
                if (segmentsIntersectNontrivial(E, vD, pu, pw, true)) return true;
                u = w;
            } while (u != ring->head());
        }

        // Cross-ring checks
        auto rings = polygon_.allRings();
        for (Ring* other : rings) {
            if (other->ringId() == ring->ringId()) continue;
            if (!other->head() || other->vertexCount() < 2) continue;

            // Check if E coincides with any vertex in other ring
            {
                Node* v = other->head();
                do {
                    if (point_eq(Vec2(v), E)) return true;
                    v = v->next;
                } while (v != other->head());
            }

            // Check edge intersections with other ring (no shared endpoint exemption)
            {
                Node* u = other->head();
                do {
                    Node* w = u->next;
                    Vec2 pu(u), pw(w);
                    if (segmentsIntersectNontrivial(vA, E, pu, pw, false)) return true;
                    if (segmentsIntersectNontrivial(E, vD, pu, pw, false)) return true;
                    u = w;
                } while (u != other->head());
            }
        }

        return false;
    }

    Polygon& polygon_;
    SpatialGrid grid_;
    CollapsePriorityQueue pq_;
    double totalDisplacement_;
    std::unordered_map<int, int> ringVertexCounts_;
};

#endif // APSC_HPP
