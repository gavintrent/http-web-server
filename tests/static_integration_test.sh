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
cat > "$CONFIG_FILE" <<EOF 
port $PORT;

location /static1 StaticHandler {
  root /static_files;
}
EOF

# Start server in background
"$SERVER_EXEC" "$CONFIG_FILE" &
SERVER_PID=$!

# Ensure server is stopped and temp files are deleted
cleanup() {
  kill $SERVER_PID 2>/dev/null || true
  rm -f "$CONFIG_FILE" "$RESPONSE_FILE"
}
trap cleanup EXIT

# give the server a moment to bind the socket
sleep 0.1

# Send GET request
curl -s -i -S "http://localhost:$PORT/static1/test.html" -o "$RESPONSE_FILE"

# Validate response
if grep -q "HTTP/1.1 200 OK" "$RESPONSE_FILE"; then
  echo "Static handler test passed."
else
  echo "Static handler test failed. Response was:"
  cat "$RESPONSE_FILE"
  exit 1
fi

# Check that Content-Type header is present
if grep -i -q "^Content-Type:" "$RESPONSE_FILE"; then
  echo "Static handler Content-Type header is present. Test passed."
else
  echo "Static handler test failed: missing Content-Type header"
  cat "$RESPONSE_FILE"
  exit 1
fi

exit 0