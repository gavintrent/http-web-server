#!/usr/bin/env bash
set -e

SERVER_EXEC="../build/bin/server"
PORT=8080

# Temp files
CONFIG_FILE=$(mktemp)
RESPONSE_FILE=$(mktemp)
OUTPUT_FILE=$(mktemp)
MESSAGES_DIR=$(mktemp -d)

# Create minimal config file
cat > "$CONFIG_FILE" <<EOF 
port $PORT;

location /messages GetMessagesHandler {
  data_path $MESSAGES_DIR;
}
EOF

# Create test messages
mkdir -p "$MESSAGES_DIR/messages"
echo '{"id":1,"text":"Hello"}' > "$MESSAGES_DIR/messages/1"
echo '{"id":2,"text":"World"}' > "$MESSAGES_DIR/messages/2"

# Start server in background
"$SERVER_EXEC" "$CONFIG_FILE" &
SERVER_PID=$!

cleanup() {
  kill $SERVER_PID 2>/dev/null || true
  rm -f "$CONFIG_FILE" "$RESPONSE_FILE" "$OUTPUT_FILE"
  rm -rf "$MESSAGES_DIR"
}
trap cleanup EXIT

# give the server a moment to bind the socket
sleep 0.1

##############################################
# 1) Test GET /messages → 200 OK + JSON array #
##############################################
echo "Testing GET /messages..."
curl -s -i -S "http://localhost:$PORT/messages" \
              -D "$RESPONSE_FILE" \
              -H "Content-Type: application/json" \
              -o "$OUTPUT_FILE"

# (a) Check HTTP status line
if grep -q "HTTP/1.1 200 OK" "$RESPONSE_FILE"; then
    echo "GET /messages - Status 200 OK"
else
    echo "GET /messages - Expected 200 OK but got:"
    cat "$RESPONSE_FILE"
    echo "Full response body:"
    cat "$OUTPUT_FILE"
    exit 1
fi

# (b) Check Content-Type header
if grep -i -q "^Content-Type: application/json" "$RESPONSE_FILE"; then
  echo "GET /messages - Content-Type: application/json"
else
  echo "GET /messages - Missing or incorrect Content-Type header"
  cat "$RESPONSE_FILE"
  exit 1
fi

# (c) Check that response body contains both JSON objects
if grep -q '"id":1' "$OUTPUT_FILE" && grep -q '"id":2' "$OUTPUT_FILE" && \
   grep -q '"text":"Hello"' "$OUTPUT_FILE" && grep -q '"text":"World"' "$OUTPUT_FILE"; then
  echo "GET /messages - Response body contains both messages."
else
  echo "GET /messages - Response body is incorrect"
  cat "$OUTPUT_FILE"
  exit 1
fi

##############################################
# 2) Test POST /messages → 405 Method Not Allowed #
##############################################
echo "Testing POST /messages..."
curl -s -i -S -X POST "http://localhost:$PORT/messages" \
              -D "$RESPONSE_FILE" \
              -H "Content-Type: application/json" \
              -o "$OUTPUT_FILE"

if grep -q "HTTP/1.1 405 Method Not Allowed" "$RESPONSE_FILE"; then
  echo "POST /messages - Method Not Allowed (405) as expected."
else
  echo "POST /messages - Expected 405 but got:"
  cat "$RESPONSE_FILE"
  exit 1
fi

if grep -i -q "^Allow: GET" "$RESPONSE_FILE"; then
  echo "POST /messages - Allow: GET header is present"
else
  echo "POST /messages - Missing or incorrect Allow header"
  cat "$RESPONSE_FILE"
  exit 1
fi

##############################################
# 3) Test GET /not-messages → 404 Not Found #
##############################################
echo "Testing GET /not-messages..."
curl -s -i -S "http://localhost:$PORT/not-messages" \
              -D "$RESPONSE_FILE" \
              -o "$OUTPUT_FILE"

if grep -q "HTTP/1.1 404 Not Found" "$RESPONSE_FILE"; then
  echo "GET /not-messages - Not Found (404) as expected."
else
  echo "GET /not-messages - Expected 404 but got:"
  cat "$RESPONSE_FILE"
  exit 1
fi

echo "All tests passed!"
exit 0
