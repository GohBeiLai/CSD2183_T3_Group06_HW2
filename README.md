# Polygon Simplification (APSC)

A high-performance C++17 implementation of the **Area-Preserving Segment Collapse (APSC)** algorithm.

## Features
- **Exact Area Preservation:** Guaranteed signed area match.
- **Topology Safety:** No self-intersections or ring-count changes.
- **High Efficiency:** Processes 400K+ vertices in < 60s.
- **Linux/WSL Optimized:** Clean `Makefile` workflow.

## Quick Start (WSL)
```bash
# 1. Enter the project directory
cd polygonHW2

# 2. Build the project
make

# 3. Run all tests
make test-all
```

## Project Structure
- `src/`: C++ header-only implementation.
- `test_cases/`: Official instructor datasets.
- `test_results/`: Logs and simplified outputs.

## Requirements
- `g++` (C++17 support)
- `make`
- WSL or Linux environment
