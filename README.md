# Polygon Simplification (APSC Algorithm)

**CSD2183 Term 3 Group 06 - Homework 2**

Implementation of the Area-Preserving Segment Collapse (APSC) algorithm for polygon simplification.

## Quick Start

### Build (Windows with Visual Studio)
```powershell
cd polygonHW2
.\build.ps1
```

### Build (Linux/Mac/MinGW)
```bash
cd polygonHW2
make clean && make
```

### Run
```bash
./simplify input.csv target_vertices
```

Example:
```bash
./simplify ../test_cases/input_blob_with_two_holes.csv 17
```

## Documentation

- **[TESTING.md](polygonHW2/TESTING.md)** - Comprehensive build, test, and validation guide
- **[test_cases/README.md](test_cases/README.md)** - Test case descriptions

## Key Features

✅ **Area Preservation** - Exact signed area maintained for each ring  
✅ **Topology Preservation** - No self-intersections, holes remain intact  
✅ **Quality Optimization** - Minimizes areal displacement (symmetric difference)  
✅ **Efficient Implementation**:
- Intrusive doubly-linked list (O(1) collapse operations)
- Lazy-deletion priority queue
- Uniform grid spatial index

## Validation Criteria

The implementation is correct when:
1. **Input area == Output area** (shown in last 3 lines of output)
2. Output vertex count ≤ target (or as close as possible)
3. No self-intersections or topology violations
4. Lower areal displacement = better quality

See [TESTING.md](polygonHW2/TESTING.md) for detailed testing instructions.

## Project Structure

```
polygonHW2/
├── src/
│   ├── main.cpp          - CLI driver
│   ├── node.hpp          - Pool-allocated node structure
│   ├── ring.hpp          - Intrusive doubly-linked list
│   ├── polygon.hpp       - Multi-ring polygon representation
│   ├── geometry.hpp      - Area calculations and intersections
│   ├── spatial_grid.hpp  - Uniform grid spatial index
│   ├── priority_queue.hpp - Lazy-deletion priority queue
│   └── apsc.hpp          - Main APSC algorithm
├── Makefile              - Build configuration (Unix)
├── build.ps1             - Build script (Windows)
└── TESTING.md            - Testing guide

test_cases/
├── README.md             - Test case descriptions
├── input_*.csv           - Test inputs
└── output_*.txt          - Expected outputs
```

## Area-Preserving Polygon Simplification (APSC Algorithm)

This project implements the Area-Preserving Segment Collapse (APSC) algorithm for polygon simplification.

### Build Instructions

Using Visual Studio:
```bash
# Open polygonHW2.sln in Visual Studio and build Release|x64
```

Using Make (Unix/MinGW):
```bash
cd polygonHW2
make clean && make
```

### Run Instructions

```bash
./simplify input.csv target_vertices
```

Or on Windows:
```bash
polygonHW2.exe input.csv target_vertices
```

### Features

- Area-preserving simplification using APSC algorithm
- Supports polygons with multiple holes (interior rings)
- Topology preservation (no self-intersections)
- Efficient spatial indexing with uniform grid
- Lazy-deletion priority queue for collapse candidates

### Output Format

```
ring_id,vertex_id,x,y
0,0,<x>,<y>
...
Total signed area in input: <value>
Total signed area in output: <value>
Total areal displacement: <value>
```

### Algorithm

The APSC algorithm iteratively collapses two consecutive segments (A→B→C→D) into a single segment (A→E→D) where E is chosen to:
1. Preserve the ring's signed area exactly
2. Minimize areal displacement (symmetric difference)

### Data Structures

1. **Intrusive Doubly Linked List**: O(1) vertex insertion/removal
2. **Lazy-Deletion Priority Queue**: Efficient collapse candidate management
3. **Uniform Grid Spatial Index**: Fast intersection detection