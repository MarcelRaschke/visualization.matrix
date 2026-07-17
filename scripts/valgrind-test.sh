#!/bin/bash
# Valgrind test script for visualization.matrix
# Usage: ./scripts/valgrind-test.sh [build-dir]

set -e

BUILD_DIR=${1:-"build"}
VALGRIND_LOG="valgrind-output.txt"
VALGRIND_OPTIONS="--leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}Running Valgrind memory analysis...${NC}"

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}Error: Build directory '$BUILD_DIR' not found.${NC}"
    echo "Please build the project first:"
    echo "  mkdir -p $BUILD_DIR && cd $BUILD_DIR && cmake -DCMAKE_BUILD_TYPE=Debug .. && make"
    exit 1
fi

# Check if executable exists
EXECUTABLE="$BUILD_DIR/visualization.matrix"
if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${RED}Error: Executable not found at $EXECUTABLE${NC}"
    exit 1
fi

echo -e "${YELLOW}Running Valgrind on $EXECUTABLE...${NC}"

# Run Valgrind
valgrind $VALGRIND_OPTIONS --log-file=$VALGRIND_LOG $EXECUTABLE 2>&1 || true

# Check results
echo -e "${YELLOW}Analyzing Valgrind output...${NC}"

if [ -f "$VALGRIND_LOG" ]; then
    # Count leaks
    DEFINITELY_LOST=$(grep -c "definitely lost:" $VALGRIND_LOG || echo "0")
    INDIRECTLY_LOST=$(grep -c "indirectly lost:" $VALGRIND_LOG || echo "0")
    POSSIBLY_LOST=$(grep -c "possibly lost:" $VALGRIND_LOG || echo "0")
    
    TOTAL_LEAKS=$((DEFINITELY_LOST + INDIRECTLY_LOST + POSSIBLY_LOST))
    
    if [ $TOTAL_LEAKS -gt 0 ]; then
        echo -e "${RED}Valgrind found $TOTAL_LEAKS memory issues:${NC}"
        echo "  Definitely lost: $DEFINITELY_LOST"
        echo "  Indirectly lost: $INDIRECTLY_LOST"
        echo "  Possibly lost: $POSSIBLY_LOST"
        echo ""
        echo -e "${RED}Full Valgrind output:${NC}"
        cat $VALGRIND_LOG
        exit 1
    else
        echo -e "${GREEN}No memory leaks detected!${NC}"
        echo -e "${GREEN}Valgrind test passed.${NC}"
        exit 0
    fi
else
    echo -e "${RED}Error: Valgrind log file not found.${NC}"
    exit 1
fi
