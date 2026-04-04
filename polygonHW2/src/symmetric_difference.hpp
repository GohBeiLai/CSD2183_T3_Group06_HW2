#ifndef SYMMETRIC_DIFFERENCE_HPP
#define SYMMETRIC_DIFFERENCE_HPP

#include <vector>
#include <cmath>
#include <algorithm>
#include <utility>

namespace symdiff {

    constexpr double SD_EPS = 1e-9;

    struct Pt { double x, y; };

    struct Edge {
        double yMin, yMax;
        double xAtYMin;
        double dxdy;
        double xAt(double y) const { return xAtYMin + (y - yMin) * dxdy; }
    };

    inline std::vector<Edge> buildEdges(const std::vector<Pt>& poly) {
        std::vector<Edge> edges;
        int n = (int)poly.size();
        for (int i = 0; i < n; i++) {
            int j = (i + 1) % n;
            double y1 = poly[i].y, y2 = poly[j].y;
            double x1 = poly[i].x, x2 = poly[j].x;
            if (std::abs(y2 - y1) < SD_EPS) continue;
            Edge e;
            if (y1 < y2) {
                e.yMin = y1; e.yMax = y2; e.xAtYMin = x1;
                e.dxdy = (x2 - x1) / (y2 - y1);
            } else {
                e.yMin = y2; e.yMax = y1; e.xAtYMin = x2;
                e.dxdy = (x1 - x2) / (y1 - y2);
            }
            edges.push_back(e);
        }
        return edges;
    }

    // Get sorted x-crossings at a given y for a set of edges
    inline std::vector<double> getCrossings(const std::vector<Edge>& edges, double y) {
        std::vector<double> xs;
        for (auto& e : edges) {
            if (y >= e.yMin && y < e.yMax) {
                xs.push_back(e.xAt(y));
            }
        }
        std::sort(xs.begin(), xs.end());
        return xs;
    }

    // Compute XOR length between two sets of even-odd crossings
    // Even-odd: inside between crossings[0]-[1], [2]-[3], etc.
    inline double computeXORLength(const std::vector<double>& xsA,
        const std::vector<double>& xsB) {
        // Build events: each crossing toggles inside/outside for its polygon
        struct Evt { double x; int poly; }; // poly: 0=A, 1=B
        std::vector<Evt> events;
        for (double x : xsA) events.push_back({ x, 0 });
        for (double x : xsB) events.push_back({ x, 1 });
        std::sort(events.begin(), events.end(), [](const Evt& a, const Evt& b) {
            return a.x < b.x;
            });

        double totalLen = 0;
        int countA = 0, countB = 0; // toggles: odd = inside
        double prevX = 0;

        for (auto& ev : events) {
            bool inA = (countA % 2) != 0;
            bool inB = (countB % 2) != 0;
            bool isXOR = inA != inB;

            if (isXOR) {
                // We're currently in XOR region; this event ends or modifies it
                totalLen += ev.x - prevX;
            }

            if (ev.poly == 0) countA++;
            else countB++;

            bool newInA = (countA % 2) != 0;
            bool newInB = (countB % 2) != 0;
            bool newXOR = newInA != newInB;

            if (newXOR) {
                prevX = ev.x;
            }
        }

        return totalLen;
    }

    inline double computeSymmetricDifference(
        const std::vector<std::pair<double, double>>& polyA,
        const std::vector<std::pair<double, double>>& polyB)
    {
        std::vector<Pt> P, Q;
        for (auto& [x, y] : polyA) P.push_back({ x, y });
        for (auto& [x, y] : polyB) Q.push_back({ x, y });

        auto edgesP = buildEdges(P);
        auto edgesQ = buildEdges(Q);

        // Collect all critical y-values
        std::vector<double> yVals;
        for (auto& p : P) yVals.push_back(p.y);
        for (auto& q : Q) yVals.push_back(q.y);

        // Add edge-edge intersection y-values
        for (auto& ep : edgesP) {
            for (auto& eq : edgesQ) {
                double yLo = std::max(ep.yMin, eq.yMin);
                double yHi = std::min(ep.yMax, eq.yMax);
                if (yLo >= yHi) continue;

                double dSlope = ep.dxdy - eq.dxdy;
                if (std::abs(dSlope) < SD_EPS) continue;

                double rhs = (eq.xAtYMin - eq.yMin * eq.dxdy) - (ep.xAtYMin - ep.yMin * ep.dxdy);
                double yInt = rhs / dSlope;

                if (yInt > yLo + SD_EPS && yInt < yHi - SD_EPS) {
                    yVals.push_back(yInt);
                }
            }
        }

        std::sort(yVals.begin(), yVals.end());
        yVals.erase(std::unique(yVals.begin(), yVals.end(),
            [](double a, double b) { return std::abs(a - b) < SD_EPS; }),
            yVals.end());

        if (yVals.size() < 2) return 0.0;

        double totalArea = 0.0;
        for (int i = 0; i + 1 < (int)yVals.size(); i++) {
            double yLo = yVals[i];
            double yHi = yVals[i + 1];
            double yMid = (yLo + yHi) * 0.5;
            double stripH = yHi - yLo;
            if (stripH < SD_EPS) continue;

            auto xsP = getCrossings(edgesP, yMid);
            auto xsQ = getCrossings(edgesQ, yMid);

            double xorLen = computeXORLength(xsP, xsQ);
            totalArea += xorLen * stripH;
        }

        return totalArea;
    }

} // namespace symdiff

#endif // SYMMETRIC_DIFFERENCE_HPP
