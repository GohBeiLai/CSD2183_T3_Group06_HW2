# Polygon Simplification (APSC Algorithm)

**CSD2183 Term 3 Group 06 - Homework 2**

Implementation of the Area-Preserving Segment Collapse (APSC) algorithm for polygon simplification.

## Quick Start

### Build (Windows with Visual Studio)
```powershell
cd polygonHW2
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\build.ps1
```

### Build (Linux/Mac/MinGW)
```bash
cd polygonHW2
make clean && make
```

### Run
```bash
./simplify <input.csv> <target_vertices>
```

Example:
```bash
./simplify ../test_cases/input_blob_with_two_holes.csv 17
```

## Dependencies

No external libraries required. Uses only the C++17 standard library.

## Documentation

- **[TESTING.md](polygonHW2/TESTING.md)** - Comprehensive build, test, and validation guide
- **[test_cases/README.md](test_cases/README.md)** - Test case descriptions

## Algorithm

The APSC algorithm iteratively collapses two consecutive segments (A to B to C to D) into a single segment (A to E to D) where E is chosen to:
1. Preserve the ring's signed area exactly
2. Minimize areal displacement (symmetric difference)

A topology check prevents any collapse that would cause self-intersection or inter-ring crossing.

## Data Structures

| Component | Data Structure | Purpose |
|-----------|---------------|---------|
| Ring representation | Intrusive doubly-linked list | O(1) vertex removal and insertion during collapse |
| Candidate selection | Lazy-deletion priority queue (`std::priority_queue`) | O(log n) extraction of minimum-displacement candidate |
| Intersection checks | Uniform grid spatial index | Near O(1) average-case segment intersection queries |
| Collapse updates | Local recomputation | Only neighbours of the collapsed segment are recomputed |

## Output Format

```
ring_id,vertex_id,x,y
0,0,<x>,<y>
...
Total signed area in input: <value>
Total signed area in output: <value>
Total areal displacement: <value>
```

The implementation is correct when input area == output area (area preserved exactly).

## Project Structure

```
polygonHW2/
├── src/
│   ├── main.cpp           - CLI driver
│   ├── node.hpp           - Pool-allocated node structure
│   ├── ring.hpp           - Intrusive doubly-linked list
│   ├── polygon.hpp        - Multi-ring polygon representation
│   ├── geometry.hpp       - Area calculations and intersections
│   ├── spatial_grid.hpp   - Uniform grid spatial index
│   ├── priority_queue.hpp - Lazy-deletion priority queue
│   ├── apsc.hpp           - Main APSC algorithm
│   └── symmetric_difference.hpp - Areal displacement computation
├── Makefile               - Build configuration (Unix)
├── build.ps1              - Build script (Windows)
└── TESTING.md             - Testing guide

test_cases/
├── README.md              - Test case descriptions
├── input_*.csv            - Test inputs (instructor-provided)
├── output_*.txt           - Expected outputs (instructor-provided)
└── test_*.csv             - Custom test inputs (experimental evaluation)

plots/
├── plot_a_runtime.png     - Running time vs input size
├── plot_b_memory.png      - Peak memory vs input size
└── plot_c_displacement.png - Areal displacement vs target vertex count
```

---

## Test Results

### Reference Test Cases (provided by instructors)

All 15 reference test cases pass: area is preserved exactly and topology is maintained.

#### Simple cases (polygons with holes)

| Test Case | Input Verts | Target | Area Preserved | Displacement |
|-----------|------------|--------|----------------|-------------|
| rectangle_with_two_holes | 12 | 7 | Yes | 1.60e+00 |
| cushion_with_hexagonal_hole | 22 | 13 | Yes | 3.84e+02 |
| blob_with_two_holes | 36 | 17 | Yes | 5.52e+04 |
| wavy_with_three_holes | 43 | 21 | Yes | 8.67e+04 |
| lake_with_two_islands | 81 | 17 | Yes | 1.05e+05 |

#### Lake cases (target: 99 vertices)

| Test Case | Input Verts | Area Preserved | Displacement |
|-----------|------------|----------------|-------------|
| original_01 | 1,860 | Yes | 8.10e+06 |
| original_02 | 8,605 | Yes | 7.91e+06 |
| original_03 | 74,559 | Yes | 9.97e+07 |
| original_04 | 6,733 | Yes | 7.51e+06 |
| original_05 | 6,230 | Yes | 4.30e+06 |
| original_06 | 14,122 | Yes | 7.97e+07 |
| original_07 | 10,596 | Yes | 1.91e+07 |
| original_08 | 6,850 | Yes | 4.70e+06 |
| original_09 | 409,998 | Yes | 1.33e+09 |
| original_10 | 9,899 | Yes | 1.31e+07 |

### Custom Test Cases

Six custom datasets designed to test specific challenging properties:

| Dataset | Input Verts | Holes | Target | Property Tested | Area Preserved |
|---------|------------|-------|--------|-----------------|----------------|
| test_many_holes_25 | 400 | 25 | 100 | Large number of holes | Yes |
| test_narrow_gap | 350 | 1 | 50 | Tight gap between exterior and hole | Yes |
| test_star_50points | 100 | 0 | 20 | Highly non-convex shape | Yes |
| test_wavy_highfreq | 2,500 | 1 | 200 | High-frequency oscillatory boundary | Yes |
| test_concentric_3holes | 750 | 3 | 100 | Large hole close to exterior | Yes |
| test_scaling_holes_50 | 1,500 | 50 | 200 | Scaling with many rings | Yes |

---

## Experimental Evaluation

### (a) Running Time vs. Input Size

Measured with target = 99 vertices on synthetic circles and real-world lakes.

| Input Vertices | Source | Time (s) |
|---------------|--------|----------|
| 1,000 | circle | 0.010 |
| 1,860 | lake 01 | 0.019 |
| 2,000 | circle | 0.017 |
| 5,000 | circle | 0.079 |
| 6,733 | lake 04 | 0.113 |
| 10,000 | circle | 0.141 |
| 14,122 | lake 06 | 0.454 |
| 20,000 | circle | 0.487 |
| 50,000 | circle | 2.461 |
| 74,559 | lake 03 | 11.798 |

**Fitted scaling:** t ≈ 1.2 × 10⁻⁷ · n^1.57

The exponent of ~1.57 places the algorithm between O(n log n) and O(n²). The super-linear factor comes from the spatial grid intersection checks during the topology test. Real-world lakes run slightly slower than circles at equivalent sizes due to geometric complexity (sinuosity, narrow inlets) causing more edges per grid cell.

![Running Time vs Input Size](plots/plot_a_runtime.png)

### (b) Peak Memory Usage vs. Input Size

| Input Vertices | Peak RSS (MB) |
|---------------|---------------|
| 1,000 | 18.1 |
| 5,000 | 16.9 |
| 10,000 | 17.8 |
| 20,000 | 18.2 |
| 50,000 | 33.4 |
| 74,559 | 43.6 |
| 100,000 | 60.0 |

**Fitted scaling:** M ≈ 0.42 KB/vertex + 14 MB base overhead → **O(n)**

Below ~20,000 vertices the 14 MB base overhead dominates. Above that, memory grows linearly at 0.42 KB per vertex, which accounts for the linked-list node, priority queue entry, and spatial grid membership per vertex.

![Peak Memory vs Input Size](plots/plot_b_memory.png)

### (c) Areal Displacement vs. Target Vertex Count

Measured on three lakes at varying simplification levels (5%–90% of vertices retained).

| Retained | Lake 01 Disp. | Lake 04 Disp. | Lake 10 Disp. |
|----------|--------------|--------------|--------------|
| 90% | 6.90e+02 | — | — |
| 50% | 2.77e+04 | 9.99e+04 | 5.31e+04 |
| 25% | 2.87e+05 | 4.10e+05 | 2.18e+05 |
| 10% | 2.97e+06 | 1.32e+06 | 9.21e+05 |
| 5% | 8.68e+06 | 2.64e+06 | 2.29e+06 |

Displacement increases exponentially as more vertices are removed. Even at 5% retention, displacement stays below 5% of the polygon's area, demonstrating the effectiveness of the area-preserving Steiner point placement.

![Areal Displacement vs Target](plots/plot_c_displacement.png)

### Summary

| Metric | Result |
|--------|--------|
| Running time scaling | O(n^1.57) |
| Memory scaling | O(n) — 0.42 KB/vertex + 14 MB base |
| Largest input tested | 74,559 vertices in 11.8 s |
| Area preservation | Exact on all 21 test cases |
| Topology preservation | Maintained on all 21 test cases |

---

## References

Kronenfeld, B. J., L. V. Stanislawski, B. P. Buttenfield, and T. Brockmeyer (2020). "Simplification of polylines by segment collapse: minimizing areal displacement while preserving area". *International Journal of Cartography* 6.1, pp. 22–46.
