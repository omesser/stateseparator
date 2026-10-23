#!/bin/bash
# Security tests for calculate.php: shell injection, input validation, PHP hardening.
# Usage: ./test_calculate_security.sh [image_name]

set -u

IMAGE="${1:-stateseparator}"
CONTAINER=""
FAILED=0

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

cleanup() {
  [[ -n "$CONTAINER" ]] && docker rm -f "$CONTAINER" > /dev/null 2>&1
}
trap cleanup EXIT

pass() { echo -e "${GREEN}PASSED${NC} $1"; }
fail() {
  echo -e "${RED}FAILED${NC} $1"
  FAILED=$((FAILED + 1))
}

CONTAINER=$(docker run -d -p 127.0.0.1::80 "$IMAGE")
PORT=$(docker port "$CONTAINER" 80/tcp | head -1 | sed 's/.*://')
URL="http://127.0.0.1:${PORT}/calculate.php"

for _ in $(seq 1 100); do
  curl -s -o /dev/null "$URL" && break
  sleep 0.1
done

MIXED=$'0.25 0 0 0\n0 0.25 0 0\n0 0 0.25 0\n0 0 0 0.25'
FIELDS=(particleSizes matrix targetDistance minProbForState targetNumberOfStates precision accuracyBoost)

# Valid value for every field; one field at a time gets a payload.
valid_value() {
  case "$1" in
    particleSizes) echo "2 2" ;;
    matrix) echo "$MIXED" ;;
    precision) echo "3" ;;
    accuracyBoost) echo "0" ;;
    *) echo "" ;;
  esac
}

# post <action> <field=value>... ; prints "<http code>" then the body to $BODY_FILE
BODY_FILE=$(mktemp)
post() {
  local action="$1"
  shift
  local args=(--data-urlencode "${action}=1")
  for kv in "$@"; do args+=(--data-urlencode "$kv"); done
  curl -s -o "$BODY_FILE" -w "%{http_code}" -X POST "$URL" "${args[@]}"
}

marker_exists() {
  docker exec "$CONTAINER" test -e "$1"
}

# --- Injection payloads must be rejected with 400 and must not execute ---
n=0
for action in separate randomize; do
  for field in "${FIELDS[@]}"; do
    # NRandomizer only receives particleSizes and precision.
    if [[ "$action" == randomize && "$field" != particleSizes && "$field" != precision ]]; then
      continue
    fi
    for style in subshell backtick semicolon; do
      n=$((n + 1))
      marker="/tmp/pwned_${n}"
      case "$style" in
        subshell) payload="\$(touch ${marker})" ;;
        backtick) payload="\`touch ${marker}\`" ;;
        semicolon) payload="\"; touch ${marker}; \"" ;;
      esac
      args=()
      for f in "${FIELDS[@]}"; do
        if [[ "$f" == "$field" ]]; then
          args+=("${f}=${payload}")
        else
          args+=("${f}=$(valid_value "$f")")
        fi
      done
      code=$(post "$action" "${args[@]}")
      name="${action}: ${style} in ${field}"
      if marker_exists "$marker"; then
        fail "$name (command executed: $marker created)"
      elif [[ "$code" != "400" ]]; then
        fail "$name (expected HTTP 400, got $code)"
      else
        pass "$name"
      fi
    done
  done
done

# --- Valid input: PHP output must match the binary run with the same argv ---
direct=$(docker exec -w /var/www/html "$CONTAINER" ./NSeparator "2 2" "$MIXED" "" "" "" "3" "0" | sed 's/[[:space:]]*$//')
code=$(post separate "particleSizes=2 2" "matrix=$MIXED" "targetDistance=" "minProbForState=" \
  "targetNumberOfStates=" "precision=3" "accuracyBoost=0")
web=$(python3 -c '
import html, re, sys
m = re.search(r"<textarea class=\"output-area\"[^>]*>(.*?)</textarea>", open(sys.argv[1]).read(), re.S)
print(html.unescape(m.group(1)) if m else "<no output area>")' "$BODY_FILE")
if [[ "$code" == "200" && "$web" == "$direct" && "$web" == *"The system is separable"* ]]; then
  pass "separate: valid input output matches direct NSeparator run"
else
  fail "separate: valid input (HTTP $code); web output differs from direct run"
  diff <(echo "$direct") <(echo "$web") | head -10
fi

# Browser textareas send CRLF; tabs and extra spaces are also valid input.
BELL=$'0.5\t0  0 0.5\r\n0 0 0 0\r\n0 0 0 0\r\n0.5 0 0 0.5'
code=$(post separate "particleSizes=2   2" "matrix=$BELL" "targetDistance=0.5E-13" "minProbForState=0" \
  "targetNumberOfStates=16" "precision=6" "accuracyBoost=1")
if [[ "$code" == "200" ]] && grep -q "The system is entangled" "$BODY_FILE"; then
  pass "separate: CRLF/tab matrix with all options set"
else
  fail "separate: CRLF/tab matrix with all options set (HTTP $code)"
fi

code=$(post randomize "particleSizes=2 2" "precision=3")
rows=$(python3 -c '
import html, re, sys
m = re.search(r"<textarea name=\"matrix\"[^>]*>(.*?)</textarea>", open(sys.argv[1]).read(), re.S)
rows = [r.split() for r in html.unescape(m.group(1)).strip().splitlines()] if m else []
print(len(rows) if rows and all(len(r) == 4 for r in rows) else 0)' "$BODY_FILE")
if [[ "$code" == "200" && "$rows" == "4" ]] && grep -q "Random matrix generated" "$BODY_FILE"; then
  pass "randomize: returns a 4x4 matrix"
else
  fail "randomize: expected a 4x4 matrix (HTTP $code, rows=$rows)"
fi

# --- PHP runtime hardening (CVE-2026-109618/109619) ---
headers=$(curl -s -D - -o /dev/null "$URL")
if echo "$headers" | grep -qi '^X-Powered-By:'; then
  fail "expose_php=Off (X-Powered-By header present)"
else
  pass "expose_php=Off (no X-Powered-By header)"
fi

upload_progress=$(docker exec "$CONTAINER" php -r 'echo ini_get("session.upload_progress.enabled") ? "on" : "off";')
if [[ "$upload_progress" == "off" ]]; then
  pass "session.upload_progress.enabled=0"
else
  fail "session.upload_progress.enabled=0 (got $upload_progress)"
fi

version=$(docker exec "$CONTAINER" php -r 'echo PHP_VERSION;')
if [[ "$(printf '%s\n' 8.2.35 "$version" | sort -V | head -1)" == "8.2.35" ]]; then
  pass "PHP >= 8.2.35 (got $version)"
else
  fail "PHP >= 8.2.35 (got $version)"
fi

rm -f "$BODY_FILE"
echo ""
if [[ "$FAILED" -eq 0 ]]; then
  echo -e "${GREEN}All security tests passed${NC}"
else
  echo -e "${RED}${FAILED} security test(s) failed${NC}"
  exit 1
fi
