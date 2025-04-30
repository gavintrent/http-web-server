#!/bin/bash

# Exit on any error
set -e

# Paths
SERVER_EXEC="../build/bin/server"
PORT=8080

# Temp files
CONFIG_FILE=$(mktemp)
RESPONSE_FILE=$(mktemp)

# Create minimal config file
echo "listen $PORT; location /echo;" > "$CONFIG_FILE"

# Start server in background
"$SERVER_EXEC" "$CONFIG_FILE" &
SERVER_PID=$!

# Ensure server is stopped and temp files are deleted
cleanup() {
  kill $SERVER_PID 2>/dev/null || true
  rm -f "$CONFIG_FILE" "$RESPONSE_FILE"
}
trap cleanup EXIT

# Give server a moment to start
sleep 1

# Send GET request
curl -s -S "http://localhost:$PORT/echo" -o "$RESPONSE_FILE"

# Validate response
if grep -q "GET /echo HTTP/1.1" "$RESPONSE_FILE" && grep -q "Host: localhost:$PORT" "$RESPONSE_FILE"; then
  echo "Echo server test passed."
  exit 0
else
  echo "Echo server test failed. Response was:"
  cat "$RESPONSE_FILE"
  exit 1
fi