#!/usr/bin/env bash
# =============================================================================
# smoketest.sh -- does this game build and run on YOUR machine?
# =============================================================================
# Run this inside the dev container:   ./smoketest.sh
#
# It builds the game, runs the tests, lets a scripted bot play all six waves,
# and prints a report at the end. Copy that report into the GitHub issue.
# Nothing here changes your machine. It is safe to run more than once.
# =============================================================================
set -uo pipefail
cd "$(dirname "$0")"

GREEN=$'\033[32m'; RED=$'\033[31m'; DIM=$'\033[2m'; OFF=$'\033[0m'
pass() { printf '%s  OK  %s%s\n' "$GREEN" "$1" "$OFF"; }
fail() { printf '%s FAIL %s%s\n' "$RED" "$1" "$OFF"; }

BUILD_OK=no; TESTS_OK=no; GAME_OK=no
TESTS_LINE="(did not run)"; SELFTEST_LINE="(did not run)"; BUILD_SECONDS="-"

echo "=============================================="
echo " Holdfast smoke test"
echo "=============================================="
echo

# --- 1. build ---------------------------------------------------------------
echo "[1/3] Building (first run downloads raylib + GoogleTest; several minutes)..."
START=$SECONDS
if cmake -S . -B build -DCMAKE_BUILD_TYPE=Release > build.log 2>&1 \
   && cmake --build build -j"$(nproc)" >> build.log 2>&1; then
    BUILD_SECONDS=$((SECONDS - START))
    BUILD_OK=yes
    pass "built in ${BUILD_SECONDS}s"
else
    fail "build failed -- last 15 lines of build.log:"
    tail -15 build.log | sed 's/^/      /'
fi
echo

# --- 2. tests (no window needed) --------------------------------------------
echo "[2/3] Running the core tests (no graphics needed)..."
if [ "$BUILD_OK" = yes ] && ./build/run_tests > tests.log 2>&1; then
    TESTS_LINE="$(grep -cE '^\[       OK \]' tests.log) passed, 0 failed"
    TESTS_OK=yes
    pass "$TESTS_LINE"
else
    TESTS_LINE=$(grep -E '^\[  FAILED  \]|tests ran' tests.log 2>/dev/null | tail -1)
    fail "tests did not pass -- see tests.log"
fi
echo

# --- 3. the game itself, on the browser desktop -----------------------------
echo "[3/3] Letting a scripted bot play all six waves..."
if [ -z "${DISPLAY:-}" ]; then
    fail "DISPLAY is not set -- are you inside the dev container?"
elif [ "$BUILD_OK" = yes ] && timeout 600 ./build/holdfast --selftest > selftest.log 2>&1; then
    SELFTEST_LINE=$(grep -E '^result=' selftest.log | tail -1)
    GAME_OK=yes
    pass "$SELFTEST_LINE"
    echo "${DIM}      expected: result=WON ... lives=19${OFF}"
else
    fail "the game did not finish -- last 10 lines of selftest.log:"
    tail -10 selftest.log 2>/dev/null | sed 's/^/      /'
fi
echo

# --- the report -------------------------------------------------------------
OS_NAME=$( (. /etc/os-release 2>/dev/null && echo "$PRETTY_NAME") || uname -s)
RENDERER=$(glxinfo -B 2>/dev/null | grep -i 'Device:' | head -1 | sed 's/^ *//' || echo "unknown (glxinfo not installed)")
VERDICT="SOMETHING FAILED -- see above"
[ "$BUILD_OK" = yes ] && [ "$TESTS_OK" = yes ] && [ "$GAME_OK" = yes ] && VERDICT="EVERYTHING WORKED"

cat <<REPORT

==============================================================
 COPY EVERYTHING BELOW THIS LINE INTO THE GITHUB ISSUE
==============================================================
**Verdict:** $VERDICT

| | |
|---|---|
| My computer | <!-- e.g. MacBook Air M2 / Dell XPS, Windows 11 / ThinkPad, Ubuntu --> |
| Container arch | $(uname -m) |
| Container OS | $OS_NAME |
| CPU cores | $(nproc) |
| Renderer | $RENDERER |
| Build | $BUILD_OK (${BUILD_SECONDS}s) |
| Tests | $TESTS_LINE |
| Bot run | $SELFTEST_LINE |

**How long did the container take to build the FIRST time?** <!-- your best guess in minutes -->

**Playing it in the browser tab at localhost:6080, how did the mouse and keyboard feel?**
<!-- Pick one and delete the rest:
     1 - unplayable, way too laggy
     2 - noticeably laggy but I could play
     3 - fine, I didn't think about it
     4 - felt completely normal -->

**Anything confusing, broken, or slow?**
<!-- Tell us. "The instructions didn't say X" is exactly what we need to hear. -->
==============================================================
REPORT
