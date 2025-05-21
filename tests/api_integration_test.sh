#!/usr/bin/env bash
set -e

SERVER_EXEC="../build/bin/server"
PORT=8080

# Temp files
CONFIG_FILE=$(mktemp)
RESPONSE_FILE=$(mktemp)
OUTPUT_FILE=$(mktemp)
API_DIR=$(mktemp -d)

# Create minimal config file
cat > "$CONFIG_FILE" <<EOF 
port $PORT;

location /api ApiHandler {
  data_path $API_DIR;
}
EOF

# Start server in background
"$SERVER_EXEC" "$CONFIG_FILE" &
SERVER_PID=$!

cleanup() {
  kill $SERVER_PID 2>/dev/null || true
  rm -f "$CONFIG_FILE" "$RESPONSE_FILE" "$OUTPUT_FILE"
  rm -rf "$API_DIR"
}
trap cleanup EXIT

# give the server a moment to bind the socket
sleep 0.1

# CREATE
REQ_DATA='{"name":"sneakers"}'
# Request on api path
curl -s -i -S -X POST "http://localhost:$PORT/api/Shoes" \
              -D  "$RESPONSE_FILE"  \
              -H "Content-Type: application/json" \
              -d "$REQ_DATA" \
              -o "$OUTPUT_FILE"

# Validate creation
if grep -q "HTTP/1.1 201 Created" "$RESPONSE_FILE"; then
  echo "CREATE - API Handler integration test passed."
else
  echo "CREATE - API Handler integration test FAILED."
  cat "$RESPONSE_FILE"
fi

# Check that Content-Type is application/json
if grep -i -q "^Content-Type: application/json" "$RESPONSE_FILE"; then
  echo "CREATE - API Handler Content-Type header is correct."
else
  echo "CREATE - Missing or incorrect Content-Type header"
  cat "$RESPONSE_FILE"
fi

# Check that response body is {"id":0}
if grep -q '{"id":0}' "$OUTPUT_FILE"; then
  echo "CREATE - API Handler response body is correct."
else
  echo "CREATE - Missing or incorrect response body"
  cat "$OUTPUT_FILE"
fi

# RETRIEVE
# Request on api path
curl -s -i -S "http://localhost:$PORT/api/Shoes/0" \
              -D "$RESPONSE_FILE"  \
              -H "Content-Type: application/json" \
              -o "$OUTPUT_FILE"

# Validate retrieval
if grep -q "HTTP/1.1 200 OK" "$RESPONSE_FILE"; then
  echo "RETRIEVE - API Handler integration test passed."
else
  echo "RETRIEVE - API Handler integration test FAILED."
  cat "$RESPONSE_FILE"
fi

# Check that Content-Type is application/json
if grep -i -q "^Content-Type: application/json" "$RESPONSE_FILE"; then
  echo "RETRIEVE - API Handler Content-Type header is correct."
else
  echo "RETRIEVE - Missing or incorrect Content-Type header"
  cat "$RESPONSE_FILE"
fi

# Check that response body is {"name":"sneakers"}
if grep -q "$REQ_DATA" "$OUTPUT_FILE"; then
  echo "RETRIEVE - API Handler response body is correct."
else
  echo "RETRIEVE - Missing or incorrect response body"
  cat "$OUTPUT_FILE"
fi

# UPDATE
NEW_REQ_DATA='{"name":"flip flops"}'
# Request on api path
curl -s -i -S -X PUT "http://localhost:$PORT/api/Shoes/0" \
              -D  "$RESPONSE_FILE"  \
              -H "Content-Type: application/json" \
              -d "$NEW_REQ_DATA" \
              -o "$OUTPUT_FILE"

# Validate retrieval
if grep -q "HTTP/1.1 200 OK" "$RESPONSE_FILE"; then
  echo "UPDATE - API Handler integration test passed."
else
  echo "UPDATE - API Handler integration test FAILED."
  cat "$RESPONSE_FILE"
fi

# Check that Content-Type is application/json
if grep -i -q "^Content-Type: application/json" "$RESPONSE_FILE"; then
  echo "UPDATE - API Handler Content-Type header is correct."
else
  echo "UPDATE - Missing or incorrect Content-Type header"
  cat "$RESPONSE_FILE"
fi

# Check that file data has been updated
curl -s -i -S "http://localhost:$PORT/api/Shoes/0" \
              -D "$RESPONSE_FILE"  \
              -H "Content-Type: application/json" \
              -o "$OUTPUT_FILE"

if grep -q "$NEW_REQ_DATA" "$OUTPUT_FILE"; then
  echo "UPDATE - API Handler updated data correctly."
else
  echo "UPDATE - Incorrect data update"
  cat "$OUTPUT_FILE"
fi

# DELETE
# Create another shoe data: /Shoes/1
curl -s -i -S -X POST "http://localhost:$PORT/api/Shoes" \
              -D  "$RESPONSE_FILE"  \
              -H "Content-Type: application/json" \
              -d "$REQ_DATA" \
              -o /dev/null

# Delete newly created shoe data
curl -s -i -S -X DELETE "http://localhost:$PORT/api/Shoes/1" \
              -D  "$RESPONSE_FILE"  \
              -H "Content-Type: application/json" \
              -o /dev/null

# Check that Content-Type is application/json
if grep -i -q "^Content-Type: application/json" "$RESPONSE_FILE"; then
  echo "DELETE - API Handler Content-Type header is correct."
else
  echo "DELETE - Missing or incorrect Content-Type header"
  cat "$RESPONSE_FILE"
fi

# Check that file data has been deleted
curl -s -i -S "http://localhost:$PORT/api/Shoes/1" \
              -D "$RESPONSE_FILE"  \
              -H "Content-Type: application/json" \
              -o /dev/null

if grep -q "HTTP/1.1 404" "$RESPONSE_FILE"; then
  echo "DELETE - API Handler succesfully deleted data."
else
  echo "DELETE - Failure: Data was NOT deleted."
  cat "$RESPONSE_FILE"
fi

# LIST
curl -s -i -S "http://localhost:$PORT/api/Shoes" \
              -D "$RESPONSE_FILE"  \
              -H "Content-Type: application/json" \
              -o "$OUTPUT_FILE"

# Validate listing IDs
if grep -q "HTTP/1.1 200 OK" "$RESPONSE_FILE"; then
  echo "LIST - API Handler integration test passed."
else
  echo "LIST - API Handler integration test FAILED."
  cat "$RESPONSE_FILE"
fi

# Check that Content-Type is application/json
if grep -i -q "^Content-Type: application/json" "$RESPONSE_FILE"; then
  echo "LIST - API Handler Content-Type header is correct."
else
  echo "LIST - Missing or incorrect Content-Type header"
  cat "$RESPONSE_FILE"
fi

# Check that response body is {"id":[0]}
if grep -q '{"id":\[0\]}' "$OUTPUT_FILE"; then
  echo "LIST - API Handler response body is correct."
else
  echo "LIST - Missing or incorrect response body"
  cat "$OUTPUT_FILE"
fi

exit 0