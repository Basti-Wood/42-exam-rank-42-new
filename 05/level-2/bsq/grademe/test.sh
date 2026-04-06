#!/bin/bash

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXAM_DIR="$(cd "$SCRIPT_DIR/../../.." && pwd)"

# Find student directory
STUDENT_DIR=""
for dir in "$EXAM_DIR/rendu/bsq" "$(dirname "$EXAM_DIR")/rendu/bsq"; do
    if [ -d "$dir" ]; then
        STUDENT_DIR="$dir"
        break
    fi
done

if [ -z "$STUDENT_DIR" ]; then
    echo -e "${RED}Error: Cannot find student directory rendu/bsq${NC}"
    exit 1
fi

# Compile
echo -e "${BLUE}Compiling BSQ...${NC}"
SRCS=$(find "$STUDENT_DIR" -name "*.c" -type f)
if [ -z "$SRCS" ]; then
    echo -e "${RED}No .c files found in $STUDENT_DIR${NC}"
    exit 1
fi

TMP="/tmp/bsq_tests_$$"
mkdir -p "$TMP"

gcc -Wall -Wextra -Werror $SRCS -I "$STUDENT_DIR" -o "$TMP/bsq" 2>"$TMP/compile_err"
if [ $? -ne 0 ]; then
    gcc -Wall -Wextra $SRCS -I "$STUDENT_DIR" -o "$TMP/bsq" 2>"$TMP/compile_err"
    if [ $? -ne 0 ]; then
        echo -e "${RED}Compilation failed${NC}"
        cat "$TMP/compile_err"
        rm -rf "$TMP"
        exit 1
    fi
    echo -e "${YELLOW}Compiled with warnings${NC}"
else
    echo -e "${GREEN}Compilation OK${NC}"
fi

BSQ="$TMP/bsq"
PASSED=0
FAILED=0
TOTAL=0

cleanup() { rm -rf "$TMP"; }
trap cleanup EXIT

# ---------- Test helper functions ----------

run_valid_test() {
    local name="$1"
    local map_file="$2"
    local expected_file="$3"
    TOTAL=$((TOTAL + 1))

    timeout 5 "$BSQ" "$map_file" > "$TMP/actual_$TOTAL" 2>/dev/null
    if diff -q "$expected_file" "$TMP/actual_$TOTAL" > /dev/null 2>&1; then
        echo -e "${GREEN}✅ Test $TOTAL: $name${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}❌ Test $TOTAL: $name${NC}"
        diff "$expected_file" "$TMP/actual_$TOTAL" 2>/dev/null | head -10 | sed 's/^/    /'
        FAILED=$((FAILED + 1))
    fi
}

run_error_test() {
    local name="$1"
    local map_file="$2"
    TOTAL=$((TOTAL + 1))

    local stderr_out
    stderr_out=$(timeout 5 "$BSQ" "$map_file" 2>&1 1>/dev/null)
    if echo "$stderr_out" | grep -q "map error"; then
        echo -e "${GREEN}✅ Test $TOTAL: $name${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}❌ Test $TOTAL: $name${NC}"
        echo "    Expected 'map error' on stderr"
        echo "    Got: '$stderr_out'"
        FAILED=$((FAILED + 1))
    fi
}

run_stdin_test() {
    local name="$1"
    local map_file="$2"
    local expected_file="$3"
    TOTAL=$((TOTAL + 1))

    timeout 5 "$BSQ" < "$map_file" > "$TMP/actual_$TOTAL" 2>/dev/null
    if diff -q "$expected_file" "$TMP/actual_$TOTAL" > /dev/null 2>&1; then
        echo -e "${GREEN}✅ Test $TOTAL: $name${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}❌ Test $TOTAL: $name${NC}"
        diff "$expected_file" "$TMP/actual_$TOTAL" 2>/dev/null | head -10 | sed 's/^/    /'
        FAILED=$((FAILED + 1))
    fi
}

echo -e "\n${BLUE}=== BSQ Tests ===${NC}\n"

# ========== TEST 1: Subject example (7x7 square) ==========
cat > "$TMP/map" << 'EOF'
9 . o x
...........................
....o......................
............o..............
...........................
....o......................
...............o...........
...........................
......o..............o.....
..o.......o................
EOF
cat > "$TMP/expected" << 'EOF'
.....xxxxxxx...............
....oxxxxxxx...............
.....xxxxxxxo..............
.....xxxxxxx...............
....oxxxxxxx...............
.....xxxxxxx...o...........
.....xxxxxxx...............
......o..............o.....
..o.......o................
EOF
run_valid_test "Subject example (7x7 square)" "$TMP/map" "$TMP/expected"

# ========== TEST 2: 5x5 empty map -> full 5x5 square ==========
cat > "$TMP/map" << 'EOF'
5 . o x
.....
.....
.....
.....
.....
EOF
cat > "$TMP/expected" << 'EOF'
xxxxx
xxxxx
xxxxx
xxxxx
xxxxx
EOF
run_valid_test "5x5 empty map (full square)" "$TMP/map" "$TMP/expected"

# ========== TEST 3: 1x1 single cell ==========
cat > "$TMP/map" << 'EOF'
1 . o x
.
EOF
cat > "$TMP/expected" << 'EOF'
x
EOF
run_valid_test "1x1 single cell" "$TMP/map" "$TMP/expected"

# ========== TEST 4: All obstacles (no square possible) ==========
cat > "$TMP/map" << 'EOF'
3 . o x
ooo
ooo
ooo
EOF
cat > "$TMP/expected" << 'EOF'
ooo
ooo
ooo
EOF
run_valid_test "All obstacles (no square)" "$TMP/map" "$TMP/expected"

# ========== TEST 5: Single row ==========
cat > "$TMP/map" << 'EOF'
1 . o x
.o..o.
EOF
cat > "$TMP/expected" << 'EOF'
xo..o.
EOF
run_valid_test "Single row (1x1 at leftmost)" "$TMP/map" "$TMP/expected"

# ========== TEST 6: Invalid map - different line lengths ==========
cat > "$TMP/map" << 'EOF'
3 . o x
.....
....
.....
EOF
run_error_test "Invalid: different line lengths" "$TMP/map"

# ========== TEST 7: Invalid map - wrong character in map ==========
cat > "$TMP/map" << 'EOF'
3 . o x
.....
..a..
.....
EOF
run_error_test "Invalid: unknown character in map" "$TMP/map"

# ========== TEST 8: Invalid map - duplicate chars ==========
cat > "$TMP/map" << 'EOF'
3 . . x
.....
.....
.....
EOF
run_error_test "Invalid: duplicate chars (empty==obstacle)" "$TMP/map"

# ========== TEST 9: Custom characters ==========
cat > "$TMP/map" << 'EOF'
3 $ @ #
$$$$$
$$$$$
$$$$$
EOF
cat > "$TMP/expected" << 'EOF'
#####
#####
#####
EOF
run_valid_test "Custom characters (full 3x3)" "$TMP/map" "$TMP/expected"

# ========== TEST 10: Checkerboard (only 1x1 possible) ==========
cat > "$TMP/map" << 'EOF'
5 . o x
o.o.o
.o.o.
o.o.o
.o.o.
o.o.o
EOF
cat > "$TMP/expected" << 'EOF'
oxo.o
.o.o.
o.o.o
.o.o.
o.o.o
EOF
run_valid_test "Checkerboard (1x1 square)" "$TMP/map" "$TMP/expected"

# ========== TEST 11: Obstacle in center of 5x5 (2x2 best) ==========
cat > "$TMP/map" << 'EOF'
5 . o x
.....
.....
..o..
.....
.....
EOF
cat > "$TMP/expected" << 'EOF'
xx...
xx...
..o..
.....
.....
EOF
run_valid_test "Obstacle in center (2x2 square)" "$TMP/map" "$TMP/expected"

# ========== TEST 12: Read map from stdin ==========
cat > "$TMP/map" << 'EOF'
3 . o x
...
...
...
EOF
cat > "$TMP/expected" << 'EOF'
xxx
xxx
xxx
EOF
run_stdin_test "Read from stdin (3x3 full)" "$TMP/map" "$TMP/expected"

# ========== TEST 13: Invalid map - missing header parameter ==========
cat > "$TMP/map" << 'EOF'
3 . x
.....
.....
.....
EOF
run_error_test "Invalid: missing obstacle char in header" "$TMP/map"

# ========== TEST 14: Wide rectangle (3x10, square is 3x3) ==========
cat > "$TMP/map" << 'EOF'
3 . o x
..........
..........
..........
EOF
cat > "$TMP/expected" << 'EOF'
xxx.......
xxx.......
xxx.......
EOF
run_valid_test "Wide rectangle (3x3 in 3x10)" "$TMP/map" "$TMP/expected"

# ========== TEST 15: Obstacle divides grid ==========
cat > "$TMP/map" << 'EOF'
5 . o x
.o...
.o...
.o...
.....
.....
EOF
cat > "$TMP/expected" << 'EOF'
.oxxx
.oxxx
.oxxx
..xxx
..xxx
EOF
run_valid_test "Obstacle column (3x3 right side)" "$TMP/map" "$TMP/expected"

# ========== RESULTS ==========
echo -e "\n${BLUE}=== Results ===${NC}"
echo -e "Passed: ${GREEN}$PASSED${NC} / $TOTAL"
echo -e "Failed: ${RED}$FAILED${NC} / $TOTAL"

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "\n${RED}Some tests failed${NC}"
    exit 1
fi
