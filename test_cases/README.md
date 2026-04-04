# Test Cases for Area-and-Topology-Preserving Polygon Simplification

Each test case consists of an **input CSV** and the corresponding **expected output**.

## Simple cases (polygons with holes)

| Input file | Vertices | Holes | Target | Output file |
|---|---|---|---|---|
| `input_rectangle_with_two_holes.csv` | 12 | 2 | 7 | `output_rectangle_with_two_holes.txt` |
| `input_cushion_with_hexagonal_hole.csv` | 22 | 1 | 13 | `output_cushion_with_hexagonal_hole.txt` |
| `input_blob_with_two_holes.csv` | 36 | 2 | 17 | `output_blob_with_two_holes.txt` |
| `input_wavy_with_three_holes.csv` | 43 | 3 | 21 | `output_wavy_with_three_holes.txt` |
| `input_lake_with_two_islands.csv` | 81 | 2 | 17 | `output_lake_with_two_islands.txt` |

## Lake cases (single polygon, no holes)

| Input file | Target | Output file |
|---|---|---|
| `input_original_01.csv` | 99 | `output_original_01.txt` |
| `input_original_02.csv` | 99 | `output_original_02.txt` |
| `input_original_03.csv` | 99 | `output_original_03.txt` |
| `input_original_04.csv` | 99 | `output_original_04.txt` |
| `input_original_05.csv` | 99 | `output_original_05.txt` |
| `input_original_06.csv` | 99 | `output_original_06.txt` |
| `input_original_07.csv` | 99 | `output_original_07.txt` |
| `input_original_08.csv` | 99 | `output_original_08.txt` |
| `input_original_09.csv` | 99 | `output_original_09.txt` |
| `input_original_10.csv` | 99 | `output_original_10.txt` |

## Custom test cases (experimental evaluation)

These datasets were created to test specific challenging properties beyond the reference cases.

| Input file | Vertices | Holes | Target | Property Tested |
|---|---|---|---|---|
| `test_many_holes_25.csv` | 400 | 25 | 100 | Large number of holes: stresses spatial index with many inter-ring intersection checks |
| `test_narrow_gap.csv` | 350 | 1 | 50 | Tight gap between exterior (r=100) and hole (r=90): collapse candidates risk inter-ring intersection |
| `test_star_50points.csv` | 100 | 0 | 20 | Highly non-convex star shape: extreme angular changes at every vertex |
| `test_wavy_highfreq.csv` | 2,500 | 1 | 200 | High-frequency sinusoidal boundary: dense small-scale detail to eliminate |
| `test_concentric_3holes.csv` | 750 | 3 | 100 | Large hole close to exterior (narrow annular gap) with 2 additional holes |
| `test_scaling_holes_50.csv` | 1,500 | 50 | 200 | 50 small holes: tests priority queue and spatial index scaling with many rings |

## Usage

```
./simplify <input_file> <target_vertices>
```

The program reads a CSV with columns `ring_id,vertex_id,x,y` and writes simplified output to stdout.
