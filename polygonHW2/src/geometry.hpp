#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <cmath>
#include <algorithm>
#include <limits>
#include <vector>
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

// Helper: Compute segment intersection parameters
// Returns true if segments (P1,P2) and (P3,P4) intersect, setting t and s
inline bool segIntersectParams(const Vec2& P1, const Vec2& P2, const Vec2& P3, const Vec2& P4,
                               double& t, double& s) {
    double dx12 = P2.x - P1.x, dy12 = P2.y - P1.y;
    double dx34 = P4.x - P3.x, dy34 = P4.y - P3.y;
    double denom = dx12 * dy34 - dy12 * dx34;
    if (std::abs(denom) < 1e-15) return false;
    double dx13 = P3.x - P1.x, dy13 = P3.y - P1.y;
    t = (dx13 * dy34 - dy13 * dx34) / denom;
    s = (dx13 * dy12 - dy13 * dx12) / denom;
    const double eps = 1e-9;
    return (t >= -eps && t <= 1.0 + eps && s >= -eps && s <= 1.0 + eps);
}

// Helper: Compute unsigned area of a polygon given its vertices
inline double polyArea(const std::vector<Vec2>& pts) {
    if (pts.size() < 3) return 0.0;
    long double acc = 0.0L;
    for (size_t i = 0, n = pts.size(); i < n; ++i) {
        const Vec2& a = pts[i];
        const Vec2& b = pts[(i + 1) % n];
        acc += static_cast<long double>(a.x) * static_cast<long double>(b.y)
             - static_cast<long double>(b.x) * static_cast<long double>(a.y);
    }
    return std::abs(static_cast<double>(acc * 0.5L));
}

// Total (unsigned) area enclosed between paths A→B→C→D and A→E→D.
//
// Area preservation guarantees L = R (left and right displacement areas are equal).
// The total displacement = L + R = 2L.
//
// We find the single intersection point I where the new path A→E→D crosses the
// old inner edge B→C (or A→E crosses B→C), split the region there, and sum the
// absolute areas of the two sub-regions.
inline double arealDisplacement(const Vec2& A, const Vec2& B, const Vec2& C, const Vec2& D, const Vec2& E) {
    double t, s;
    const double epsCross = 1e-10;

    // Check if E→D crosses B→C
    if (segIntersectParams(E, D, B, C, t, s)) {
        if (!(t <= epsCross || t >= 1.0 - epsCross || s <= epsCross || s >= 1.0 - epsCross)) {
            Vec2 I(E.x + t * (D.x - E.x), E.y + t * (D.y - E.y));
            double a1 = triangleArea(A, B, I) + triangleArea(A, I, E);
            double a2 = triangleArea(I, C, D) + triangleArea(I, D, E);
            return std::abs(a1) + std::abs(a2);
        }
    }

    // Check if A→E crosses B→C
    if (segIntersectParams(A, E, B, C, t, s)) {
        if (!(t <= epsCross || t >= 1.0 - epsCross || s <= epsCross || s >= 1.0 - epsCross)) {
            Vec2 I(A.x + t * (E.x - A.x), A.y + t * (E.y - A.y));
            double a1 = std::abs(triangleArea(A, B, I));
            double a2 = std::abs(triangleArea(I, C, D) + triangleArea(I, D, E));
            return a1 + a2;
        }
    }

    // Check if E→D crosses A→B
    if (segIntersectParams(E, D, A, B, t, s)) {
        if (!(t <= epsCross || t >= 1.0 - epsCross || s <= epsCross || s >= 1.0 - epsCross)) {
            Vec2 I(E.x + t * (D.x - E.x), E.y + t * (D.y - E.y));
            double a1 = std::abs(triangleArea(A, I, E));
            double a2 = polyArea({I, B, C, D, E});
            return a1 + a2;
        }
    }

    // Check if A→E crosses C→D
    if (segIntersectParams(A, E, C, D, t, s)) {
        if (!(t <= epsCross || t >= 1.0 - epsCross || s <= epsCross || s >= 1.0 - epsCross)) {
            Vec2 J(A.x + t * (E.x - A.x), A.y + t * (E.y - A.y));
            double a1 = std::abs(triangleArea(J, D, E));
            double a2 = polyArea({A, B, C, J, E});
            return a1 + a2;
        }
    }

    // No intersection case: use shoelace for the pentagon ABCDE
    double area = (A.x * B.y - B.x * A.y)
                + (B.x * C.y - C.x * B.y)
                + (C.x * D.y - D.x * C.y)
                + (D.x * E.y - E.x * D.y)
                + (E.x * A.y - A.x * E.y);
    double shoelace_disp = std::abs(area) * 0.5;
    
    // Alternative calculations for robustness
    double alt1 = std::abs(triangleArea(A, B, C) + triangleArea(A, C, E))
                + std::abs(triangleArea(C, D, E));
    double alt2 = std::abs(triangleArea(A, B, E))
                + std::abs(triangleArea(B, C, D) + triangleArea(B, D, E));
    double alt3 = std::abs(triangleArea(A, B, E)) + std::abs(triangleArea(E, C, D));
    
    return std::max(std::max(shoelace_disp, alt1), std::max(alt2, alt3));
}

// Intersection of the line a*x + b*y + c = 0 with the infinite line through P1 and P2.
// Returns the intersection point.
inline Vec2 intersectEstarWithLine(double a, double b, double c, const Vec2& P1, const Vec2& P2) {
    double dx = P2.x - P1.x;
    double dy = P2.y - P1.y;
    double denom = a * dx + b * dy;
    if (std::abs(denom) < 1e-15) {
        // Lines are parallel: project midpoint onto E*
        Vec2 mid = (P1 + P2) * 0.5;
        double n2 = a * a + b * b;
        double val = a * mid.x + b * mid.y + c;
        return Vec2(mid.x - a * val / n2, mid.y - b * val / n2);
    }
    double t = -(a * P1.x + b * P1.y + c) / denom;
    return Vec2(P1.x + t * dx, P1.y + t * dy);
}

// APSC: Given vertices A, B, C, D where we collapse B and C to a new point E,
// compute E such that the ring's signed area is preserved.
// The collapse goes: ...A-B-C-D... -> ...A-E-D...
//
// Implements the APSC placement function from Kronenfeld et al. (2020), Section 3.
//
// Derivation:
//   The area-preserving constraint (eq. 1b) defines a line E*:
//     a*xE + b*yE + c = 0
//   where
//     a = yD - yA
//     b = xA - xD
//     c = -yB*xA + (yA-yC)*xB + (yB-yD)*xC + yC*xD
//
//   E* is parallel to AD. Any E on E* exactly preserves the ring's signed area.
//
//   Among all points on E*, the one that minimises areal displacement is:
//     - If B and C are on the SAME side of AD:
//         take the intersection of E* with AB (if B is farther from AD),
//         or with CD (if C is farther from AD).
//     - If B and C are on OPPOSITE sides of AD:
//         take the intersection of E* with AB (if B is on the same side as E*),
//         or with CD otherwise.
inline Vec2 computeAreaPreservingPoint(const Vec2& A, const Vec2& B, 
                                        const Vec2& C, const Vec2& D,
                                        double& displacement) {
    // --- Compute E* line: a*x + b*y + c = 0 ---
    double a = D.y - A.y;
    double b = A.x - D.x;
    double c = -B.y * A.x
             + (A.y - C.y) * B.x
             + (B.y - D.y) * C.x
             + C.y * D.x;

    // Degenerate: A == D (zero-length base)
    double ad2 = (D.x - A.x) * (D.x - A.x) + (D.y - A.y) * (D.y - A.y);
    if (ad2 < 1e-18) {
        Vec2 E((B.x + C.x) * 0.5, (B.y + C.y) * 0.5);
        displacement = arealDisplacement(A, B, C, D, E);
        return E;
    }

    // Degenerate: E* coincides with line AD (every point on AD preserves area)
    // Detected by checking that A lies on E*.
    double val_A = a * A.x + b * A.y + c;
    double scale = std::abs(a) + std::abs(b) + 1.0;
    if (std::abs(val_A) < 1e-10 * scale) {
        // Any point on AD is optimal; return midpoint.
        Vec2 E((A.x + D.x) * 0.5, (A.y + D.y) * 0.5);
        displacement = arealDisplacement(A, B, C, D, E);
        return E;
    }

    // Find E by intersecting E* with both AB and CD, pick the one with lower displacement
    Vec2 Eab = intersectEstarWithLine(a, b, c, A, B);
    Vec2 Ecd = intersectEstarWithLine(a, b, c, C, D);
    double dab = arealDisplacement(A, B, C, D, Eab);
    double dcd = arealDisplacement(A, B, C, D, Ecd);
    
    if (dab <= dcd) {
        displacement = dab;
        return Eab;
    } else {
        displacement = dcd;
        return Ecd;
    }
}

// --- Robust intersection helpers (matching Project 2's geometry.cpp) ---

inline bool point_eq(const Vec2& a, const Vec2& b) {
    const double eps = 1e-12;
    return std::abs(a.x - b.x) <= eps && std::abs(a.y - b.y) <= eps;
}

inline int orient_sign(const Vec2& a, const Vec2& b, const Vec2& c) {
    long double abx = static_cast<long double>(b.x) - static_cast<long double>(a.x);
    long double aby = static_cast<long double>(b.y) - static_cast<long double>(a.y);
    long double acx = static_cast<long double>(c.x) - static_cast<long double>(a.x);
    long double acy = static_cast<long double>(c.y) - static_cast<long double>(a.y);
    long double v   = abx * acy - aby * acx;
    long double tol = 1e-12L * (fabsl(abx) + fabsl(aby) + fabsl(acx) + fabsl(acy) + 1.0L);
    if (fabsl(v) <= tol) return 0;
    return (v > 0) ? 1 : -1;
}

inline bool on_segment_robust(const Vec2& a, const Vec2& b, const Vec2& p) {
    const double eps = 1e-12;
    if (orient_sign(a, b, p) != 0) return false;
    return (p.x >= std::min(a.x, b.x) - eps && p.x <= std::max(a.x, b.x) + eps &&
            p.y >= std::min(a.y, b.y) - eps && p.y <= std::max(a.y, b.y) + eps);
}

inline bool collinear_overlap_nontrivial(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d) {
    if (orient_sign(a, b, c) != 0 || orient_sign(a, b, d) != 0) return false;
    const double eps = 1e-12;
    const double abx = std::abs(b.x - a.x);
    const double aby = std::abs(b.y - a.y);
    auto proj = [&](const Vec2& p) -> double { return (abx >= aby) ? p.x : p.y; };
    double a0 = proj(a), a1 = proj(b);
    double c0 = proj(c), c1 = proj(d);
    if (a0 > a1) std::swap(a0, a1);
    if (c0 > c1) std::swap(c0, c1);
    double overlap = std::min(a1, c1) - std::max(a0, c0);
    return overlap > eps;
}

// Matches Project 2's segments_intersect_nontrivial:
// Returns true if segments (p1,p2) and (p3,p4) intersect in a non-trivial way.
// When ignore_shared_endpoints is true, a touch at a shared endpoint is not counted.
inline bool segmentsIntersectNontrivial(const Vec2& p1, const Vec2& p2,
                                         const Vec2& p3, const Vec2& p4,
                                         bool ignore_shared_endpoints) {
    const bool shared_endpoint =
        point_eq(p1, p3) || point_eq(p1, p4) || point_eq(p2, p3) || point_eq(p2, p4);

    if (collinear_overlap_nontrivial(p1, p2, p3, p4)) return true;

    int o1 = orient_sign(p1, p2, p3);
    int o2 = orient_sign(p1, p2, p4);
    int o3 = orient_sign(p3, p4, p1);
    int o4 = orient_sign(p3, p4, p2);

    if (o1 * o2 < 0 && o3 * o4 < 0) return true;

    bool intersects =
        (o1 == 0 && on_segment_robust(p1, p2, p3)) ||
        (o2 == 0 && on_segment_robust(p1, p2, p4)) ||
        (o3 == 0 && on_segment_robust(p3, p4, p1)) ||
        (o4 == 0 && on_segment_robust(p3, p4, p2));

    if (!intersects) return false;
    if (ignore_shared_endpoints && shared_endpoint) return false;
    return true;
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
