#!/usr/bin/env bash
set -e

SERVER_EXEC="../build/bin/server"
PORT=8080

# Temp files
CONFIG_FILE=$(mktemp)
RESPONSE_FILE=$(mktemp)
OUTPUT_FILE=$(mktemp)

# Create minimal config file that wires /health to HealthHandler
cat > "$CONFIG_FILE" <<EOF
port $PORT;

location /health HealthHandler {}
EOF

# Start server in background
"$SERVER_EXEC" "$CONFIG_FILE" &
SERVER_PID=$!

cleanup() {
  kill $SERVER_PID 2>/dev/null || true
  rm -f "$CONFIG_FILE" "$RESPONSE_FILE" "$OUTPUT_FILE"
}
trap cleanup EXIT

# give the server a moment to bind the socket
sleep 0.1

echo "==== HEALTH CHECK ===="
# Valid GET /health
curl -s -i "http://localhost:$PORT/health" \
     -o "$RESPONSE_FILE"

# Status line
if grep -q "HTTP/1.1 200 OK" "$RESPONSE_FILE"; then
  echo "PASS: health returns 200 OK"
else
  echo "FAIL: health did not return 200 OK"
  cat "$RESPONSE_FILE"
  exit 1
fi

# Content-Type header
if grep -i -q "^Content-Type: text/plain" "$RESPONSE_FILE"; then
  echo "PASS: Content-Type is text/plain"
else
  echo "FAIL: incorrect or missing Content-Type"
  cat "$RESPONSE_FILE"
  exit 1
fi

# Body exactly "OK"
BODY=$(awk 'BEGIN {RS="\r\n\r\n"} NR==2 {print}' "$RESPONSE_FILE")
if [[ "$BODY" == "OK" ]]; then
  echo "PASS: body is exactly 'OK'"
else
  echo "FAIL: body is not 'OK' (got: '$BODY')"
  cat "$RESPONSE_FILE"
  exit 1
fi

echo "==== MALFORMED REQUEST ===="
# Send a malformed request and expect 400 Bad Request
# Use a raw socket via bash built-in /dev/tcp
exec 3<>/dev/tcp/localhost/$PORT
printf "THIS IS NOT HTTP\r\n\r\n" >&3
cat <&3 > "$RESPONSE_FILE"
exec 3<&-
exec 3>&-

# Check for 400 response
if grep -q "HTTP/1.1 400 Bad Request" "$RESPONSE_FILE"; then
  echo "PASS: malformed request returns 400 Bad Request"
else
  echo "FAIL: malformed request did not return 400"
  cat "$RESPONSE_FILE"
  exit 1
fi

exit 0
