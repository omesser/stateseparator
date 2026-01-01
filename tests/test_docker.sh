#!/bin/bash
# Docker integration tests for State Separator
# Usage: ./test_docker.sh [image_name]

set -e

IMAGE="${1:-stateseparator}"
CONTAINER_NAME="stateseparator-test-$$"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

cleanup() {
    echo "Cleaning up..."
    docker stop "$CONTAINER_NAME" 2>/dev/null || true
    docker rm "$CONTAINER_NAME" 2>/dev/null || true
}

trap cleanup EXIT

echo "========================================"
echo "State Separator - Docker Integration Tests"
echo "========================================"
echo ""
echo "Using image: $IMAGE"
echo ""

# Start container
echo "Starting container..."
docker run -d -p 8081:80 --name "$CONTAINER_NAME" "$IMAGE" > /dev/null

# Wait for Apache to start
echo "Waiting for Apache to start..."
sleep 3

# Test 1: Check container is running
echo -n "Test: Container is running... "
if docker ps | grep -q "$CONTAINER_NAME"; then
    echo -e "${GREEN}PASSED${NC}"
else
    echo -e "${RED}FAILED${NC}"
    exit 1
fi

# Test 2: Check binaries are ELF executables
echo -n "Test: NSeparator is ELF executable... "
BINARY_TYPE=$(docker exec "$CONTAINER_NAME" file /var/www/html/NSeparator)
if echo "$BINARY_TYPE" | grep -q "ELF"; then
    echo -e "${GREEN}PASSED${NC}"
else
    echo -e "${RED}FAILED${NC}"
    echo "  Got: $BINARY_TYPE"
    exit 1
fi

# Test 3: Run separator directly in container
echo -n "Test: NSeparator runs in container... "
MATRIX=$'0.25 0 0 0\n0 0.25 0 0\n0 0 0.25 0\n0 0 0 0.25'
OUTPUT=$(docker exec "$CONTAINER_NAME" /var/www/html/NSeparator "2 2" "$MATRIX" "" "" "" "3" "0" 2>&1)
if echo "$OUTPUT" | grep -q "The system is separable"; then
    echo -e "${GREEN}PASSED${NC}"
else
    echo -e "${RED}FAILED${NC}"
    echo "  Output: $OUTPUT"
    exit 1
fi

# Test 4: Check HTTP server responds
echo -n "Test: HTTP server responds... "
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:8081/)
if [[ "$HTTP_CODE" == "200" ]]; then
    echo -e "${GREEN}PASSED${NC}"
else
    echo -e "${RED}FAILED${NC}"
    echo "  Got HTTP code: $HTTP_CODE"
    exit 1
fi

# Test 5: Check index.html contains expected content
echo -n "Test: Index page contains State Separator... "
CONTENT=$(curl -s http://localhost:8081/)
if echo "$CONTENT" | grep -q "State Separator"; then
    echo -e "${GREEN}PASSED${NC}"
else
    echo -e "${RED}FAILED${NC}"
    exit 1
fi

# Test 6: Check PHP is working
echo -n "Test: PHP is processing calculate.php... "
CONTENT=$(curl -s http://localhost:8081/calculate.php)
if echo "$CONTENT" | grep -q "density matrix"; then
    echo -e "${GREEN}PASSED${NC}"
else
    echo -e "${RED}FAILED${NC}"
    exit 1
fi

# Test 7: Test form submission via curl
echo -n "Test: Form submission works... "
RESULT=$(curl -s -X POST http://localhost:8081/calculate.php \
    -d "separate=1" \
    -d "particleSizes=2 2" \
    -d "matrix=0.25 0 0 0
0 0.25 0 0
0 0 0.25 0
0 0 0 0.25" \
    -d "targetDistance=" \
    -d "minProbForState=" \
    -d "targetNumberOfStates=" \
    -d "precision=3" \
    -d "accuracyBoost=0")

if echo "$RESULT" | grep -q "separable"; then
    echo -e "${GREEN}PASSED${NC}"
else
    echo -e "${RED}FAILED${NC}"
    echo "  Result:"
    echo "$RESULT" | head -20
    exit 1
fi

# Test 8: Test Bell state (entangled)
echo -n "Test: Bell state correctly identified as entangled... "
RESULT=$(curl -s -X POST http://localhost:8081/calculate.php \
    -d "separate=1" \
    -d "particleSizes=2 2" \
    -d "matrix=0.5 0 0 0.5
0 0 0 0
0 0 0 0
0.5 0 0 0.5" \
    -d "targetDistance=" \
    -d "minProbForState=" \
    -d "targetNumberOfStates=" \
    -d "precision=3" \
    -d "accuracyBoost=0")

if echo "$RESULT" | grep -qi "entangled"; then
    echo -e "${GREEN}PASSED${NC}"
else
    echo -e "${RED}FAILED${NC}"
    echo "  Result:"
    echo "$RESULT" | grep -i "system\|separable\|entangled" | head -5
    exit 1
fi

echo ""
echo "========================================"
echo -e "${GREEN}All Docker integration tests passed!${NC}"
echo "========================================"
