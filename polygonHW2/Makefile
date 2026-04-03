# Makefile for Area-and-Topology-Preserving Polygon Simplification

# Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3

# Directories
SRCDIR = src
TARGET = simplify

# Files
SOURCES = $(SRCDIR)/main.cpp
HEADERS = $(wildcard $(SRCDIR)/*.hpp)

# Build Target
all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

# Clean
clean:
	rm -f $(TARGET)
	rm -f *.o
	rm -f bin/*.o

# Helper to extract value from summary line
# Usage: $(call get_val,filename,pattern)
define get_val
grep "$(2)" $(1) | cut -d: -f2 | sed 's/^[[:space:]]*//;s/[[:space:]]*$$//'
endef

# Run a single test case
test-case: $(TARGET)
	@mkdir -p ../test_results
	@echo "----------------------------------------------------------"
	@echo "Testing $(IN) (Target: $(TARGET_VERTICES))"
	@./$(TARGET) ../test_cases/$(IN) $(TARGET_VERTICES) > ../test_results/actual_$(EXPECT) 2> ../test_results/log_$(IN).txt
	@echo "--- Comparison Summary ---"
	@printf "%-30s | %-20s | %-20s\n" "Metric" "Sample (Expected)" "Actual (Yours)"
	@printf "%-30s | %-20s | %-20s\n" "------------------------------" "--------------------" "--------------------"
	@S_IN=$$($(call get_val,../test_cases/$(EXPECT),Total signed area in input)); \
	 A_IN=$$($(call get_val,../test_results/actual_$(EXPECT),Total signed area in input)); \
	 printf "%-30s | %-20s | %-20s\n" "Total Signed Area (Input)" "$$S_IN" "$$A_IN"
	@S_OUT=$$($(call get_val,../test_cases/$(EXPECT),Total signed area in output)); \
	 A_OUT=$$($(call get_val,../test_results/actual_$(EXPECT),Total signed area in output)); \
	 printf "%-30s | %-20s | %-20s\n" "Total Signed Area (Output)" "$$S_OUT" "$$A_OUT"
	@S_DISP=$$($(call get_val,../test_cases/$(EXPECT),Total areal displacement)); \
	 A_DISP=$$($(call get_val,../test_results/actual_$(EXPECT),Total areal displacement)); \
	 printf "%-30s | %-20s | %-20s\n" "Total Areal Displacement" "$$S_DISP" "$$A_DISP"
	@V_COUNT=$$(grep -c "^[0-9]" ../test_results/actual_$(EXPECT)); \
	 printf "%-30s | %-20s | %-20s\n" "Final Vertex Count" "$(TARGET_VERTICES)" "$$V_COUNT"

# Run all 15 test cases
test-all: $(TARGET)
	@echo "=== RUNNING ALL TEST CASES ==="
	@$(MAKE) -s test-case IN=input_rectangle_with_two_holes.csv TARGET_VERTICES=11 EXPECT=output_rectangle_with_two_holes.txt
	@$(MAKE) -s test-case IN=input_cushion_with_hexagonal_hole.csv TARGET_VERTICES=13 EXPECT=output_cushion_with_hexagonal_hole.txt
	@$(MAKE) -s test-case IN=input_blob_with_two_holes.csv TARGET_VERTICES=17 EXPECT=output_blob_with_two_holes.txt
	@$(MAKE) -s test-case IN=input_wavy_with_three_holes.csv TARGET_VERTICES=21 EXPECT=output_wavy_with_three_holes.txt
	@$(MAKE) -s test-case IN=input_lake_with_two_islands.csv TARGET_VERTICES=17 EXPECT=output_lake_with_two_islands.txt
	@for i in 01 02 03 04 05 06 07 08 09 10; do \
		$(MAKE) -s test-case IN=input_original_$$i.csv TARGET_VERTICES=99 EXPECT=output_original_$$i.txt; \
	done
	@echo "----------------------------------------------------------"
	@echo "Tests complete. Results stored in 'test_results/'"
