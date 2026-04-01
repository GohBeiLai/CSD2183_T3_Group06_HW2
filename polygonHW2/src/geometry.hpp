#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <cmath>
#include <algorithm>
#include <limits>
#include "node.hpp"

constexpr double EPSILON = 1e-10;
constexpr double AREA_TOLERANCE = 1e-9;

// 2D vector/point operations
struct Vec2 {
    double x, y;
    
    Vec2() : x(0), y(0) {}
    Vec2(double x_, double y_) : x(x_), y(y_) {}
    Vec2(const Node* n) : x(n->x), y(n->y) {}
    
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(double s) const { return {x * s, y * s}; }
    Vec2 operator/(double s) const { return {x / s, y / s}; }
    
    double dot(const Vec2& o) const { return x * o.x + y * o.y; }
    double cross(const Vec2& o) const { return x * o.y - y * o.x; }
    double length() const { return std::sqrt(x * x + y * y); }
    double lengthSq() const { return x * x + y * y; }
    
    Vec2 normalized() const {
        double len = length();
        return len > EPSILON ? Vec2(x / len, y / len) : Vec2(0, 0);
    }
};

// Signed area of triangle (positive if CCW)
inline double triangleArea(const Vec2& a, const Vec2& b, const Vec2& c) {
    return 0.5 * ((b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y));
}

inline double triangleArea(const Node* a, const Node* b, const Node* c) {
    return triangleArea(Vec2(a), Vec2(b), Vec2(c));
}

// Signed area of polygon ring using shoelace formula
inline double ringSignedArea(const Node* start) {
    if (!start) return 0.0;
    
    double area = 0.0;
    const Node* curr = start;
    do {
        area += curr->x * curr->next->y - curr->next->x * curr->y;
        curr = curr->next;
    } while (curr != start);
    
    return area * 0.5;
}

// Count vertices in a ring
inline int countVertices(const Node* start) {
    if (!start) return 0;
    int count = 0;
    const Node* curr = start;
    do {
        count++;
        curr = curr->next;
    } while (curr != start);
    return count;
}

// Check if point is strictly left of line from a to b
inline double signedArea2(const Vec2& a, const Vec2& b, const Vec2& c) {
    return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
}

// Orientation of three points: positive = CCW, negative = CW, 0 = collinear
inline double orientation(const Vec2& a, const Vec2& b, const Vec2& c) {
    return signedArea2(a, b, c);
}

// Check if two segments (p1,p2) and (p3,p4) intersect properly (not at endpoints)
inline bool segmentsIntersectProperly(const Vec2& p1, const Vec2& p2, 
                                       const Vec2& p3, const Vec2& p4) {
    double d1 = orientation(p3, p4, p1);
    double d2 = orientation(p3, p4, p2);
    double d3 = orientation(p1, p2, p3);
    double d4 = orientation(p1, p2, p4);
    
    // Check for proper intersection (segments cross each other)
    if (((d1 > EPSILON && d2 < -EPSILON) || (d1 < -EPSILON && d2 > EPSILON)) &&
        ((d3 > EPSILON && d4 < -EPSILON) || (d3 < -EPSILON && d4 > EPSILON))) {
        return true;
    }
    
    return false;
}

// Check if point p is on segment (a, b)
inline bool pointOnSegment(const Vec2& a, const Vec2& b, const Vec2& p) {
    if (std::abs(orientation(a, b, p)) > EPSILON) return false;
    return p.x >= std::min(a.x, b.x) - EPSILON && p.x <= std::max(a.x, b.x) + EPSILON &&
           p.y >= std::min(a.y, b.y) - EPSILON && p.y <= std::max(a.y, b.y) + EPSILON;
}

// Check if segments intersect (including touching at endpoints)
inline bool segmentsIntersect(const Vec2& p1, const Vec2& p2, 
                              const Vec2& p3, const Vec2& p4) {
    double d1 = orientation(p3, p4, p1);
    double d2 = orientation(p3, p4, p2);
    double d3 = orientation(p1, p2, p3);
    double d4 = orientation(p1, p2, p4);
    
    if (((d1 > EPSILON && d2 < -EPSILON) || (d1 < -EPSILON && d2 > EPSILON)) &&
        ((d3 > EPSILON && d4 < -EPSILON) || (d3 < -EPSILON && d4 > EPSILON))) {
        return true;
    }
    
    // Collinear cases
    if (std::abs(d1) <= EPSILON && pointOnSegment(p3, p4, p1)) return true;
    if (std::abs(d2) <= EPSILON && pointOnSegment(p3, p4, p2)) return true;
    if (std::abs(d3) <= EPSILON && pointOnSegment(p1, p2, p3)) return true;
    if (std::abs(d4) <= EPSILON && pointOnSegment(p1, p2, p4)) return true;
    
    return false;
}

// Compute intersection point of lines (not segments) through (p1,p2) and (p3,p4)
inline Vec2 lineIntersection(const Vec2& p1, const Vec2& p2, 
                             const Vec2& p3, const Vec2& p4) {
    double a1 = p2.y - p1.y;
    double b1 = p1.x - p2.x;
    double c1 = a1 * p1.x + b1 * p1.y;
    
    double a2 = p4.y - p3.y;
    double b2 = p3.x - p4.x;
    double c2 = a2 * p3.x + b2 * p3.y;
    
    double det = a1 * b2 - a2 * b1;
    if (std::abs(det) < EPSILON) {
        // Lines are parallel, return midpoint as fallback
        return (p1 + p2 + p3 + p4) / 4.0;
    }
    
    return Vec2((b2 * c1 - b1 * c2) / det, (a1 * c2 - a2 * c1) / det);
}

// Calculate the area of the quadrilateral ABCD (signed)
inline double quadrilateralArea(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d) {
    return triangleArea(a, b, c) + triangleArea(a, c, d);
}

// APSC: Given vertices A, B, C, D where we collapse B and C to a new point E,
// compute E such that the ring's signed area is preserved.
// The collapse goes: ...A-B-C-D... -> ...A-E-D...
// 
// The ring's signed area changes when we replace edges A-B, B-C, C-D with A-E, E-D.
// Using shoelace formula, each edge (p1, p2) contributes (p1.x*p2.y - p2.x*p1.y).
// 
// Old contribution (×2): (A.x*B.y - B.x*A.y) + (B.x*C.y - C.x*B.y) + (C.x*D.y - D.x*C.y)
// New contribution (×2): (A.x*E.y - E.x*A.y) + (E.x*D.y - D.x*E.y)
//
// Simplifying new contrib: A.x*E.y - E.x*A.y + E.x*D.y - D.x*E.y
//                        = E.x*(D.y - A.y) + E.y*(A.x - D.x)
//
// For area preservation: E.x*(D.y - A.y) + E.y*(A.x - D.x) = oldContrib
// This is a line equation for E!
//
// To find E, we project the midpoint of BC onto this line perpendicular to BC,
// or we find the point on this area-preserving line closest to B-C segment.
inline Vec2 computeAreaPreservingPoint(const Vec2& a, const Vec2& b, 
                                        const Vec2& c, const Vec2& d,
                                        double& displacement) {
    // Compute old edge contributions
    double oldContrib = (a.x * b.y - b.x * a.y) + 
                        (b.x * c.y - c.x * b.y) + 
                        (c.x * d.y - d.x * c.y);
    
    // Coefficients of the area-preserving line: coefX * E.x + coefY * E.y = oldContrib
    double coefX = d.y - a.y;
    double coefY = a.x - d.x;
    
    // Midpoint of B and C
    Vec2 mid = (b + c) * 0.5;
    
    // Direction along BC
    Vec2 bc = c - b;
    double bcLen = bc.length();
    
    // If BC has zero length, just use B
    if (bcLen < EPSILON) {
        // E must satisfy: coefX * E.x + coefY * E.y = oldContrib
        // Project B onto this line
        double norm = coefX * coefX + coefY * coefY;
        if (norm < EPSILON) {
            displacement = 0;
            return b;
        }
        double dist = (coefX * b.x + coefY * b.y - oldContrib) / norm;
        Vec2 e = Vec2(b.x - dist * coefX, b.y - dist * coefY);
        displacement = std::abs(triangleArea(b, c, e));
        return e;
    }
    
    // We want E on the area-preserving line, as close as possible to segment BC
    // The area-preserving line: coefX * x + coefY * y = oldContrib
    // Normal to this line: (coefX, coefY)
    
    // Option 1: Project midpoint of BC onto the area-preserving line
    double norm = coefX * coefX + coefY * coefY;
    if (norm < EPSILON) {
        // Degenerate case: any point works
        displacement = 0;
        return mid;
    }
    
    // Distance from midpoint to the line (signed)
    double distMid = (coefX * mid.x + coefY * mid.y - oldContrib) / std::sqrt(norm);
    
    // Project midpoint onto the line
    Vec2 normal = Vec2(coefX, coefY).normalized();
    Vec2 e = mid - normal * distMid;
    
    // Verify: coefX * e.x + coefY * e.y should equal oldContrib
    // (debugging check, can be removed)
    
    // Calculate displacement
    // The displacement is the area "swept" by moving from the original edges to new edges
    // Simplified: area of triangle BCE (the region between original BC and new point E)
    displacement = std::abs(triangleArea(b, c, e));
    
    // If E is exactly on line BC, displacement might be near zero
    // In that case, use the change in triangle areas as displacement metric
    if (displacement < EPSILON) {
        double areaABC = triangleArea(a, b, c);
        double areaBCD = triangleArea(b, c, d);
        double areaAED = triangleArea(a, e, d);
        // The actual shape difference
        displacement = std::abs(areaABC + areaBCD - areaAED);
    }
    
    return e;
}

// Bounding box helper
struct BBox {
    double minX, minY, maxX, maxY;
    
    BBox() : minX(std::numeric_limits<double>::max()),
             minY(std::numeric_limits<double>::max()),
             maxX(std::numeric_limits<double>::lowest()),
             maxY(std::numeric_limits<double>::lowest()) {}
    
    void expand(double x, double y) {
        minX = std::min(minX, x);
        minY = std::min(minY, y);
        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
    }
    
    void expand(const Vec2& p) { expand(p.x, p.y); }
    void expand(const Node* n) { expand(n->x, n->y); }
    
    double width() const { return maxX - minX; }
    double height() const { return maxY - minY; }
    
    bool intersects(const BBox& other) const {
        return !(maxX < other.minX || minX > other.maxX ||
                 maxY < other.minY || minY > other.maxY);
    }
};

// Get bounding box of a segment
inline BBox segmentBBox(const Vec2& a, const Vec2& b) {
    BBox box;
    box.expand(a);
    box.expand(b);
    return box;
}

#endif // GEOMETRY_HPP
