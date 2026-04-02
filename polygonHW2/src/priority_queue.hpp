#ifndef PRIORITY_QUEUE_HPP
#define PRIORITY_QUEUE_HPP

#include <queue>
#include <vector>
#include <functional>
#include "node.hpp"
#include "geometry.hpp"

// Collapse candidate: represents collapsing B and C to a new point E
// A -> B -> C -> D becomes A -> E -> D
struct CollapseCandidate {
    Node* a;        // First vertex (stays)
    Node* b;        // Second vertex (to be removed)
    Node* c;        // Third vertex (to be removed)
    Node* d;        // Fourth vertex (stays)
    
    Vec2 newE;      // Computed position for E
    double displacement;  // Area displaced by this collapse
    
    uint64_t genA, genB, genC, genD;  // Generation counters for lazy deletion
    int ring_id;
    
    CollapseCandidate() : a(nullptr), b(nullptr), c(nullptr), d(nullptr),
                          displacement(std::numeric_limits<double>::max()),
                          genA(0), genB(0), genC(0), genD(0), ring_id(-1) {}
    
    CollapseCandidate(Node* a_, Node* b_, Node* c_, Node* d_, int ring_id_)
        : a(a_), b(b_), c(c_), d(d_), ring_id(ring_id_) {
        genA = a->generation;
        genB = b->generation;
        genC = c->generation;
        genD = d->generation;
        
        // Compute the area-preserving point and displacement
        newE = computeAreaPreservingPoint(Vec2(a), Vec2(b), Vec2(c), Vec2(d), displacement);
    }
    
    // Check if this candidate is still valid
    // A candidate is stale if any vertex has been modified (generation changed)
    // or if the adjacency A→B→C→D no longer holds in the ring.
    bool isValid() const {
        if (!a || !b || !c || !d) return false;
        
        // Check generation counters (for modified/removed vertices)
        if (a->generation != genA || b->generation != genB ||
            c->generation != genC || d->generation != genD) {
            return false;
        }
        
        // Check adjacency is still valid
        if (a->next != b) return false;
        if (b->next != c) return false;
        if (c->next != d) return false;
        
        return true;
    }
    
    // For priority queue: lower displacement = higher priority
    // Uses deterministic tie-breaking based on vertex IDs (same as reference implementation)
    bool operator<(const CollapseCandidate& other) const {
        const double eps = 1e-12;
        double diff = displacement - other.displacement;
        if (std::fabs(diff) > eps) return diff > 0.0;  // Min-heap: larger values have lower priority
        if (ring_id != other.ring_id) return ring_id > other.ring_id;
        
        // Tie-break using original vertex IDs for deterministic ordering
        auto id = [](Node* v) { return v ? v->original_id : -1; };
        int aA = id(a), aB = id(b), aC = id(c), aD = id(d);
        int bA = id(other.a), bB = id(other.b), bC = id(other.c), bD = id(other.d);
        if (aB != bB) return aB > bB;
        if (aC != bC) return aC > bC;
        if (aA != bA) return aA > bA;
        if (aD != bD) return aD > bD;
        return a > other.a;
    }
};

// Lazy-deletion priority queue for collapse candidates
class CollapsePriorityQueue {
public:
    CollapsePriorityQueue() : validCount_(0), totalCount_(0) {}
    
    // Add a candidate to the queue
    void push(const CollapseCandidate& candidate) {
        pq_.push(candidate);
        totalCount_++;
        validCount_++;
    }
    
    // Get the best valid candidate (skip stale entries)
    bool pop(CollapseCandidate& result) {
        while (!pq_.empty()) {
            CollapseCandidate top = pq_.top();
            pq_.pop();
            totalCount_--;
            
            if (top.isValid()) {
                result = top;
                validCount_--;
                return true;
            }
            // Stale entry, skip it
            validCount_--;  // Was counted as valid when pushed
        }
        return false;
    }
    
    // Check if queue is empty (no valid candidates)
    bool empty() const {
        return pq_.empty();
    }
    
    // Get approximate valid count (may include stale entries)
    size_t size() const {
        return pq_.size();
    }
    
    // Rebuild queue if too many stale entries (optional optimization)
    void compact() {
        if (totalCount_ > 1000 && validCount_ * 3 < totalCount_) {
            std::vector<CollapseCandidate> valid;
            while (!pq_.empty()) {
                CollapseCandidate c = pq_.top();
                pq_.pop();
                if (c.isValid()) {
                    valid.push_back(c);
                }
            }
            for (const auto& c : valid) {
                pq_.push(c);
            }
            totalCount_ = valid.size();
            validCount_ = valid.size();
        }
    }
    
    // Clear the queue
    void clear() {
        while (!pq_.empty()) pq_.pop();
        validCount_ = 0;
        totalCount_ = 0;
    }

private:
    std::priority_queue<CollapseCandidate> pq_;
    size_t validCount_;
    size_t totalCount_;
};

#endif // PRIORITY_QUEUE_HPP
