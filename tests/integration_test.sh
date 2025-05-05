#!/bin/bash

# Exit on any error
set -e

# Paths
SERVER_EXEC="../build/bin/server"
PORT=8080

# Temp files
CONFIG_FILE=$(mktemp)
RESPONSE_BODY=$(mktemp)
RESPONSE_HEADERS=$(mktemp)

# Create minimal config file
echo "listen $PORT; location /echo;" > "$CONFIG_FILE"

# Start server in background
"$SERVER_EXEC" "$CONFIG_FILE" &
SERVER_PID=$!

# Ensure server is stopped and temp files are deleted
cleanup() {
  kill $SERVER_PID 2>/dev/null || true
  rm -f "$CONFIG_FILE" "$RESPONSE_BODY" "$RESPONSE_HEADERS"
}
trap cleanup EXIT

# Send GET request, capturing headers and body separately
curl -s -D "$RESPONSE_HEADERS" "http://localhost:$PORT/echo" -o "$RESPONSE_BODY"

# Validate response body contains echoed request
if grep -q "GET /echo HTTP/1.1" "$RESPONSE_BODY" && grep -q "Host: localhost:$PORT" "$RESPONSE_BODY"; then
  echo "Echo server body test passed."
else
  echo "Echo server body test failed. Response body was:"
  cat "$RESPONSE_BODY"
  exit 1
fi

# Check that Content-Type header is present
if grep -i -q "^Content-Type:" "$RESPONSE_HEADERS"; then
  echo "Echo handler Content-Type header is present. Test passed."
else
  echo "Echo handler test failed: missing Content-Type header"
  echo "Response headers were:"
  cat "$RESPONSE_HEADERS"
  exit 1
fi

exit 0