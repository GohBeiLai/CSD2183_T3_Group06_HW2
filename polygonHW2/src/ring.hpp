#ifndef RING_HPP
#define RING_HPP

#include <vector>
#include "node.hpp"
#include "geometry.hpp"

// Ring class using intrusive doubly linked list
class Ring {
public:
    Ring(int ring_id, NodePool& pool) : ring_id_(ring_id), head_(nullptr), 
                                         vertex_count_(0), next_generated_id_(0), pool_(pool) {}
    
    ~Ring() = default;  // Pool manages node memory
    
    // Add vertex to ring (builds the linked list)
    void addVertex(double x, double y, int orig_id = -1) {
        Node* node = pool_.allocate(x, y, ring_id_, orig_id);
        
        // Track the max original_id for generating new IDs
        if (orig_id >= next_generated_id_) {
            next_generated_id_ = orig_id + 1;
        }
        
        if (!head_) {
            head_ = node;
            node->prev = node;
            node->next = node;
        } else {
            // Insert before head (at end of ring)
            Node* tail = head_->prev;
            tail->next = node;
            node->prev = tail;
            node->next = head_;
            head_->prev = node;
        }
        vertex_count_++;
    }
    
    // Get vertex count
    int vertexCount() const { return vertex_count_; }
    
    // Get head node
    Node* head() const { return head_; }
    
    // Get ring ID
    int ringId() const { return ring_id_; }
    
    // Get the next generated ID (for assigning to new/repurposed vertices)
    int nextGeneratedId() { return next_generated_id_++; }
    
    // Remove a node from the ring (does not deallocate)
    void removeNode(Node* node) {
        if (!node || vertex_count_ == 0) return;
        
        if (vertex_count_ == 1) {
            head_ = nullptr;
        } else {
            node->prev->next = node->next;
            node->next->prev = node->prev;
            if (node == head_) {
                head_ = node->next;
            }
        }
        vertex_count_--;
        node->generation++;  // Invalidate any references in priority queue
    }
    
    // Insert a new node after the given node
    Node* insertAfter(Node* after, double x, double y) {
        Node* newNode = pool_.allocate(x, y, ring_id_, next_generated_id_++);
        
        newNode->next = after->next;
        newNode->prev = after;
        after->next->prev = newNode;
        after->next = newNode;
        
        vertex_count_++;
        return newNode;
    }
    
    // Replace node's position (for area-preserving collapse)
    void updatePosition(Node* node, double x, double y) {
        node->x = x;
        node->y = y;
        node->generation++;
    }
    
    // Calculate signed area of ring
    double signedArea() const {
        return ringSignedArea(head_);
    }
    
    // Check if ring is valid (at least 3 vertices for a polygon)
    bool isValid() const {
        return vertex_count_ >= 3;
    }
    
    // Update head if it was removed
    void setHead(Node* newHead) {
        head_ = newHead;
    }
    
    // Iterate over all vertices
    template<typename Func>
    void forEachVertex(Func&& func) const {
        if (!head_) return;
        Node* curr = head_;
        do {
            func(curr);
            curr = curr->next;
        } while (curr != head_);
    }
    
    // Iterate over all edges (pairs of consecutive vertices)
    template<typename Func>
    void forEachEdge(Func&& func) const {
        if (!head_) return;
        Node* curr = head_;
        do {
            func(curr, curr->next);
            curr = curr->next;
        } while (curr != head_);
    }
    
    // Get all vertices as a vector (for output)
    std::vector<std::pair<double, double>> getVertices() const {
        std::vector<std::pair<double, double>> vertices;
        vertices.reserve(vertex_count_);
        forEachVertex([&](const Node* n) {
            vertices.emplace_back(n->x, n->y);
        });
        return vertices;
    }

private:
    int ring_id_;
    Node* head_;
    int vertex_count_;
    int next_generated_id_;  // For assigning IDs to new/repurposed vertices
    NodePool& pool_;
};

#endif // RING_HPP
