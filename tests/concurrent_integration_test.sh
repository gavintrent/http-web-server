#!/usr/bin/env bash
set -e

SERVER_EXEC="../build/bin/server"
PORT=8080

# Temp files
CONFIG_FILE=$(mktemp)
RESPONSE_FILE=$(mktemp)

# Create minimal config that wires /sleep and /echo
cat > "$CONFIG_FILE" <<EOF
port $PORT;

location /sleep SleepHandler {}

location /echo EchoHandler {}
EOF

# Start server in background
"$SERVER_EXEC" "$CONFIG_FILE" &
SERVER_PID=$!

cleanup() {
  kill $SERVER_PID 2>/dev/null || true
  rm -f "$CONFIG_FILE" "$RESPONSE_FILE"
}
trap cleanup EXIT

# Allow server time to start
sleep 0.3

echo "==== CONCURRENT REQUEST TEST ===="

# Launch /sleep in background
curl -s "http://localhost:$PORT/sleep" > /dev/null &
SLEEP_PID=$!

# Record start time and send /echo
START=$(date +%s.%N)
curl -s "http://localhost:$PORT/echo" -o "$RESPONSE_FILE"
END=$(date +%s.%N)

# Compute elapsed time for /echo
ELAPSED=$(echo "$END - $START" | bc)

echo "Time for /echo: $ELAPSED seconds"

# Evaluate result
if (( $(echo "$ELAPSED < 2.0" | bc -l) )); then
  echo "PASS: /echo completed quickly while /sleep was blocking"
else
  echo "FAIL: /echo was blocked (elapsed: $ELAPSED)"
  cat "$RESPONSE_FILE"
  exit 1
fi
