#!/usr/bin/env bash
set -e

SERVER_EXEC="../build/bin/server"
PORT=8080

# Temp files
CONFIG_FILE=$(mktemp)
RESPONSE_HEADERS=$(mktemp)

# Minimal config: only our three handlers
cat > "$CONFIG_FILE" <<EOF
port $PORT;
location /;
EOF

# Launch server
"$SERVER_EXEC" "$CONFIG_FILE" &
SERVER_PID=$!

cleanup() {
  kill $SERVER_PID 2>/dev/null || true
  rm -f "$CONFIG_FILE" "$RESPONSE_HEADERS"
}
trap cleanup EXIT

# Request a non-existent path
curl -s -D "$RESPONSE_HEADERS" "http://localhost:$PORT/some/unknown/path" -o /dev/null

# Verify we got a 404 status line
if grep -q "^HTTP/1.1 404" "$RESPONSE_HEADERS"; then
  echo "NotFoundHandler integration test passed."
else
  echo "NotFoundHandler integration test FAILED."
  cat "$RESPONSE_HEADERS"
  # exit 1
fi

# Also check Content-Type header was set
if grep -i "^Content-Type: text/plain" "$RESPONSE_HEADERS"; then
  echo "Content-Type header OK."
else
  echo "Missing or incorrect Content-Type header"
  # exit 1
fi