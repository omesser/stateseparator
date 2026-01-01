#!/bin/bash
# Basic sanity tests for State Separator
# Usage: ./test_separator.sh [path_to_NSeparator] [path_to_NRandomizer]
#
# NOTE: On macOS, POSIX semaphores (sem_init) are deprecated and may cause
# "Failed to initialize a semaphore" errors. Use Docker tests instead:
#   ./test_docker.sh stateseparator
#
# This script works best inside the Docker container:
#   docker exec <container> /tests/test_separator.sh /var/www/html/NSeparator /var/www/html/NRandomizer

# Don't exit on first error - we want to run all tests
set +e

NSEPARATOR="${1:-./NSeparator}"
NRANDOMIZER="${2:-./NRandomizer}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

TESTS_PASSED=0
TESTS_FAILED=0

# Helper function to run a test
run_test() {
  local test_name="$1"
  local expected_pattern="$2"
  shift 2
  local cmd=("$@")

  echo -n "Testing: $test_name... "

  # Capture output, ignoring exit codes
  local output=""
  output=$("${cmd[@]}" 2>&1) || true

  if echo "$output" | grep -q "$expected_pattern"; then
    echo -e "${GREEN}PASSED${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
    return 0
  else
    echo -e "${RED}FAILED${NC}"
    echo "  Expected pattern: $expected_pattern"
    echo "  Got output (first 5 lines):"
    echo "$output" | sed -n '1,5p' | sed 's/^/    /'
    TESTS_FAILED=$((TESTS_FAILED + 1))
    return 1
  fi
}

echo "========================================"
echo "State Separator - Sanity Tests"
echo "========================================"
echo ""

# Check if binaries exist
if [[ ! -x "$NSEPARATOR" ]]; then
  echo -e "${RED}ERROR: NSeparator not found or not executable at: $NSEPARATOR${NC}"
  exit 1
fi

if [[ ! -x "$NRANDOMIZER" ]]; then
  echo -e "${YELLOW}WARNING: NRandomizer not found at: $NRANDOMIZER${NC}"
fi

echo "Using NSeparator: $NSEPARATOR"
echo "Using NRandomizer: $NRANDOMIZER"
echo ""

# =============================================================================
# Test 1: Maximally Mixed State (Separable)
# =============================================================================
MATRIX_MIXED=$'0.25 0 0 0\n0 0.25 0 0\n0 0 0.25 0\n0 0 0 0.25'

run_test "Maximally mixed state (2x2) is separable" \
  "The system is separable" \
  "$NSEPARATOR" "2 2" "$MATRIX_MIXED" "" "" "" "3" "0"

# =============================================================================
# Test 2: Bell State (Entangled)
# =============================================================================
MATRIX_BELL=$'0.5 0 0 0.5\n0 0 0 0\n0 0 0 0\n0.5 0 0 0.5'

run_test "Bell state (|00⟩+|11⟩)/√2 is entangled" \
  "entangled" \
  "$NSEPARATOR" "2 2" "$MATRIX_BELL" "" "" "" "3" "0"

# =============================================================================
# Test 3: Pure Product State (Separable)
# =============================================================================
# |00⟩⟨00| = [[1,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0]]
MATRIX_PRODUCT=$'1 0 0 0\n0 0 0 0\n0 0 0 0\n0 0 0 0'

run_test "Pure product state |00⟩ is separable" \
  "The system is separable" \
  "$NSEPARATOR" "2 2" "$MATRIX_PRODUCT" "" "" "" "3" "0"

# =============================================================================
# Test 4: Different precision levels
# =============================================================================
run_test "Precision 6 works" \
  "The system is separable" \
  "$NSEPARATOR" "2 2" "$MATRIX_MIXED" "" "" "" "6" "0"

run_test "Precision 9 works" \
  "The system is separable" \
  "$NSEPARATOR" "2 2" "$MATRIX_MIXED" "" "" "" "9" "0"

# =============================================================================
# Test 5: 3-particle system (2x2x2 = 8 dimensional)
# =============================================================================
# Maximally mixed state for 3 qubits
MATRIX_3QUBIT=$'0.125 0 0 0 0 0 0 0
0 0.125 0 0 0 0 0 0
0 0 0.125 0 0 0 0 0
0 0 0 0.125 0 0 0 0
0 0 0 0 0.125 0 0 0
0 0 0 0 0 0.125 0 0
0 0 0 0 0 0 0.125 0
0 0 0 0 0 0 0 0.125'

run_test "3-qubit maximally mixed state is separable" \
  "The system is separable" \
  "$NSEPARATOR" "2 2 2" "$MATRIX_3QUBIT" "" "" "" "3" "0"

# =============================================================================
# Test 6: Error handling - invalid particle sizes
# =============================================================================
run_test "Invalid particle sizes rejected" \
  "ERROR" \
  "$NSEPARATOR" "abc" "$MATRIX_MIXED" "" "" "" "3" "0"

# =============================================================================
# Test 7: Error handling - matrix size mismatch
# =============================================================================
run_test "Matrix size mismatch rejected" \
  "ERROR" \
  "$NSEPARATOR" "2 2" "1 0 0 1" "" "" "" "3" "0"

# =============================================================================
# Test 8: NRandomizer generates valid output
# =============================================================================
if [[ -x "$NRANDOMIZER" ]]; then
  run_test "NRandomizer generates matrix for 2x2 system" \
    "^[0-9.-]" \
    "$NRANDOMIZER" "2 2" "3"
fi

# =============================================================================
# Summary
# =============================================================================
echo ""
echo "========================================"
echo "Test Summary"
echo "========================================"
echo -e "Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Failed: ${RED}$TESTS_FAILED${NC}"
echo ""

if [[ $TESTS_FAILED -gt 0 ]]; then
  echo -e "${RED}Some tests failed!${NC}"
  exit 1
else
  echo -e "${GREEN}All tests passed!${NC}"
  exit 0
fi
