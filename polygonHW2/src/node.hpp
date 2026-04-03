#ifndef NODE_HPP
#define NODE_HPP

#include <vector>
#include <cstdint>
#include <memory>

// Forward declaration
struct Node;

// Node in an intrusive doubly linked list representing a polygon ring vertex
struct Node {
    double x, y;
    Node* prev;
    Node* next;
    int ring_id;
    int original_id;      // Original vertex ID from input (for deterministic ordering)
    uint64_t generation;  // For lazy deletion in priority queue
    
    Node() : x(0), y(0), prev(nullptr), next(nullptr), ring_id(-1), original_id(-1), generation(0) {}
    Node(double x_, double y_, int ring_id_ = -1, int orig_id_ = -1) 
        : x(x_), y(y_), prev(nullptr), next(nullptr), ring_id(ring_id_), original_id(orig_id_), generation(0) {}
};

// Pool allocator for nodes - avoids fragmentation and improves cache locality
class NodePool {
public:
    explicit NodePool(size_t initial_capacity = 1024) {
        nodes_.reserve(initial_capacity);
    }
    
    // Allocate a new node
    Node* allocate(double x, double y, int ring_id = -1, int orig_id = -1) {
        if (!free_list_.empty()) {
            Node* node = free_list_.back();
            free_list_.pop_back();
            node->x = x;
            node->y = y;
            node->ring_id = ring_id;
            node->original_id = orig_id;
            node->prev = nullptr;
            node->next = nullptr;
            node->generation++;
            return node;
        }
        nodes_.push_back(std::make_unique<Node>(x, y, ring_id, orig_id));
        return nodes_.back().get();
    }
    
    // Return node to pool
    void deallocate(Node* node) {
        if (node) {
            node->prev = nullptr;
            node->next = nullptr;
            node->generation++;
            free_list_.push_back(node);
        }
    }
    
    // Clear all nodes
    void clear() {
        nodes_.clear();
        free_list_.clear();
    }

private:
    std::vector<std::unique_ptr<Node>> nodes_;
    std::vector<Node*> free_list_;
};

#endif // NODE_HPP
