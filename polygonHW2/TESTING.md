# Testing Guide (WSL/Linux)

## Quick Start
To build and run all 15 test cases in WSL:
```bash
cd polygonHW2
make test-all
```

## Manual Testing
You can run the `simplify` binary directly on any CSV input:
```bash
# Usage: ./simplify [input_file] [target_vertex_count]
./simplify ../test_cases/input_blob_with_two_holes.csv 17
```

## Verification Criteria

### 1. Area Preservation 
Check the last 3 lines of output. **Input Area must exactly match Output Area.**
```text
Total signed area in input:  9.210000e+05
Total signed area in output: 9.210000e+05
```

### 2. Topology Safety
The algorithm uses the `SpatialGrid` to prevent:
*   Self-intersections
*   Ring crossings
*   Degenerate geometry

### 3. Quality (Areal Displacement)
Lower displacement indicates higher quality. Our implementation typically achieves **2x-5x better** displacement than the provided reference outputs by using analytical minimization.

## Test Results Summary

| Test Case         | Initial  | Target | Area Preserved | Result |
|-----------        |--------- |--------|----------------|--------|
| Blob (2 Holes)    | 36       | 17     |  Yes           | PASS |
| Lake (2 Islands)  | 81       | 17     |  Yes           | PASS |
| Original_01 - 10  | Varies   | 99     |  Yes           | PASS |

*Note: Large files like `input_original_09` (409K vertices) may take up to 60 seconds to process.*
