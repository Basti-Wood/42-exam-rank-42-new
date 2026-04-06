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
for dir in "$EXAM_DIR/rendu/game_of_life" "$(dirname "$EXAM_DIR")/rendu/game_of_life"; do
    if [ -d "$dir" ]; then
        STUDENT_DIR="$dir"
        break
    fi
done

if [ -z "$STUDENT_DIR" ]; then
    echo -e "${RED}Error: Cannot find student directory rendu/game_of_life${NC}"
    exit 1
fi

# Compile
echo -e "${BLUE}Compiling Game of Life...${NC}"
SRCS=$(find "$STUDENT_DIR" -name "*.c" -type f)
if [ -z "$SRCS" ]; then
    echo -e "${RED}No .c files found in $STUDENT_DIR${NC}"
    exit 1
fi

TMP="/tmp/life_tests_$$"
mkdir -p "$TMP"

gcc -Wall -Wextra -Werror $SRCS -I "$STUDENT_DIR" -o "$TMP/life" 2>"$TMP/compile_err"
if [ $? -ne 0 ]; then
    gcc -Wall -Wextra $SRCS -I "$STUDENT_DIR" -o "$TMP/life" 2>"$TMP/compile_err"
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

LIFE="$TMP/life"
PASSED=0
FAILED=0
TOTAL=0

cleanup() { rm -rf "$TMP"; }
trap cleanup EXIT

# ---------- Test helper ----------
# Each line of output is exactly <width> chars wide, <height> lines total.
# Live cells = '0', dead cells = ' '

run_test() {
    local name="$1"
    local input="$2"
    local width="$3"
    local height="$4"
    local iterations="$5"
    local expected_file="$6"
    TOTAL=$((TOTAL + 1))

    printf '%s' "$input" | timeout 5 "$LIFE" "$width" "$height" "$iterations" > "$TMP/actual_$TOTAL" 2>/dev/null
    if diff -q "$expected_file" "$TMP/actual_$TOTAL" > /dev/null 2>&1; then
        echo -e "${GREEN}✅ Test $TOTAL: $name${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}❌ Test $TOTAL: $name${NC}"
        echo "    Expected:"
        cat "$expected_file" | cat -A | head -8 | sed 's/^/      /'
        echo "    Got:"
        cat "$TMP/actual_$TOTAL" | cat -A | head -8 | sed 's/^/      /'
        FAILED=$((FAILED + 1))
    fi
}

echo -e "\n${BLUE}=== Game of Life Tests ===${NC}\n"

# ========== TEST 1: Subject example 1 - hollow square, 0 iterations ==========
# echo 'sdxddssaaww' | ./a.out 5 5 0
# Pen trace: (0,0)→s(1,0)→d(1,1)→x[DOWN,mark(1,1)]→d(1,2)→d(1,3)→s(2,3)→s(3,3)→a(3,2)→a(3,1)→w(2,1)→w(1,1)
# Marked: (1,1)(1,2)(1,3)(2,1)(2,3)(3,1)(3,2)(3,3)
cat > "$TMP/expected" << 'EOF'
     
 000 
 0 0 
 000 
     
EOF
run_test "Subject ex1: hollow square 5x5 iter=0" "sdxddssaaww" 5 5 0 "$TMP/expected"

# ========== TEST 2: Subject example 2 - two shapes, 0 iterations ==========
# echo 'sdxssdswdxdddxsaddawxwdxwaa' | ./a.out 10 6 0
cat > "$TMP/expected" << 'EOF'
          
 0   000  
 0     0  
 000  0   
  0  000  
          
EOF
run_test "Subject ex2: two shapes 10x6 iter=0" "sdxssdswdxdddxsaddawxwdxwaa" 10 6 0 "$TMP/expected"

# ========== TEST 3: Subject example 3 - vertical line, 0 iterations ==========
# echo 'dxss' | ./a.out 3 3 0
# Pen: (0,0)→d(0,1)→x[mark(0,1)]→s(1,1)→s(2,1)
cat > "$TMP/expected" << 'EOF'
 0 
 0 
 0 
EOF
run_test "Subject ex3: vertical line 3x3 iter=0" "dxss" 3 3 0 "$TMP/expected"

# ========== TEST 4: Subject example 4 - blinker after 1 iteration ==========
# Vertical line → horizontal line
cat > "$TMP/expected" << 'EOF'
   
000
   
EOF
run_test "Subject ex4: blinker 3x3 iter=1" "dxss" 3 3 1 "$TMP/expected"

# ========== TEST 5: Subject example 5 - blinker after 2 iterations ==========
# Back to vertical
cat > "$TMP/expected" << 'EOF'
 0 
 0 
 0 
EOF
run_test "Subject ex5: blinker 3x3 iter=2" "dxss" 3 3 2 "$TMP/expected"

# ========== TEST 6: Block (still life) stable after 10 iterations ==========
# Draw 2x2 block at (1,1): s→d→x[mark(1,1)]→d(1,2)→s(2,2)→a(2,1)
cat > "$TMP/expected" << 'EOF'
    
 00 
 00 
    
EOF
run_test "Block still life 4x4 iter=10" "sdxdsaw" 4 4 10 "$TMP/expected"

# ========== TEST 7: Empty board, 0 iterations ==========
cat > "$TMP/expected" << 'EOF'
   
   
   
EOF
run_test "Empty board 3x3 iter=0" "" 3 3 0 "$TMP/expected"

# ========== TEST 8: Single cell dies after 1 iteration ==========
# Draw at (1,1): s→d→x[mark(1,1)]
cat > "$TMP/expected" << 'EOF'
   
   
   
EOF
run_test "Single cell dies 3x3 iter=1" "sdx" 3 3 1 "$TMP/expected"

# ========== TEST 9: Pen boundary - can't move off-grid ==========
# 'wwwxs' on 3x3: w×3 stays at (0,0), x marks (0,0), s marks (1,0)
cat > "$TMP/expected" << 'EOF'
0  
0  
   
EOF
run_test "Pen boundary (stays at edge)" "wwwxs" 3 3 0 "$TMP/expected"

# ========== TEST 10: Full row, 0 iterations ==========
# Draw row 1: s→x[mark(1,0)]→d(1,1)→d(1,2)→d(1,3)→d(1,4)
cat > "$TMP/expected" << 'EOF'
     
00000
     
EOF
run_test "Full row 5x3 iter=0" "sxdddd" 5 3 0 "$TMP/expected"

# ========== TEST 11: Horizontal blinker → vertical after 1 iter ==========
# Draw horizontal at row 2: ss→d→x[mark(2,1)]→d(2,2)→d(2,3)
# After 1 iter: becomes vertical at col 2, rows 1-3
cat > "$TMP/expected" << 'EOF'
     
  0  
  0  
  0  
     
EOF
run_test "Horiz blinker→vertical 5x5 iter=1" "ssdxdd" 5 5 1 "$TMP/expected"

# ========== TEST 12: Invalid commands are ignored ==========
# 'dxzs1s' should behave like 'dxss' (z and 1 are ignored)
cat > "$TMP/expected" << 'EOF'
 0 
 0 
 0 
EOF
run_test "Invalid cmds ignored (z,1)" "dxzs1s" 3 3 0 "$TMP/expected"

# ========== TEST 13: Pen toggle off and on ==========
# 'xddxddxdd' on 7x1: x[mark(0,0)]→d(0,1)→d(0,2)→x[UP]→d(0,3)→d(0,4)→x[mark(0,4)]→d(0,5)→d(0,6)
cat > "$TMP/expected" << 'EOF'
000  00
EOF
run_test "Pen toggle off/on 7x1 iter=0" "xddxddxdd" 7 1 0 "$TMP/expected"

# ========== TEST 14: Blinker period check - 4 iterations = back to start ==========
# 4 iterations of blinker (period 2) → even → same as 0
cat > "$TMP/expected" << 'EOF'
 0 
 0 
 0 
EOF
run_test "Blinker after 4 iter (even=original)" "dxss" 3 3 4 "$TMP/expected"

# ========== TEST 15: Larger board with glider-like shape ==========
# Draw L-shape + dot: creates interesting evolution
# 'sdxdssdx' on 6x6 iter=0
# s(1,0)→d(1,1)→x[mark(1,1)]→d(1,2)→s(2,2)→s(3,2)→d(3,3)→x[UP]
cat > "$TMP/expected" << 'EOF'
      
 00   
  0   
  0   
      
      
EOF
run_test "L-shape 6x6 iter=0" "sdxdssdx" 6 6 0 "$TMP/expected"

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
