#!/usr/bin/env bash
set -e

SERVER_EXEC="../build/bin/server"
PORT=8080

# Temp files
CONFIG_FILE=$(mktemp)
RESPONSE_FILE=$(mktemp)

# Create minimal config file that wires /logout to LogoutHandler
cat > "$CONFIG_FILE" <<EOF
port $PORT;

location /logout LogoutHandler {}
EOF

# Start server in background
"$SERVER_EXEC" "$CONFIG_FILE" &
SERVER_PID=$!

cleanup() {
  kill $SERVER_PID 2>/dev/null || true
  rm -f "$CONFIG_FILE" "$RESPONSE_FILE"
}
trap cleanup EXIT

# give the server a moment to bind the socket
sleep 0.1

echo "==== LOGOUT CHECK ===="
# Valid GET /logout
curl -s -i "http://localhost:$PORT/logout" -o "$RESPONSE_FILE"

# Status line
if grep -q "HTTP/1.1 200 OK" "$RESPONSE_FILE"; then
  echo "PASS: logout returns 200 OK"
else
  echo "FAIL: logout did not return 200 OK"
  cat "$RESPONSE_FILE"
  exit 1
fi

# Set-Cookie header (expired session cookie)
if grep -q -i "^Set-Cookie: session=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT; HttpOnly" "$RESPONSE_FILE"; then
  echo "PASS: Set-Cookie header clears session"
else
  echo "FAIL: incorrect or missing Set-Cookie header"
  grep -i "^Set-Cookie" "$RESPONSE_FILE" || cat "$RESPONSE_FILE"
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

# Body exactly "Logged out successfully"
BODY=$(awk 'BEGIN {RS="\r\n\r\n"} NR==2 {print}' "$RESPONSE_FILE")
if [[ "$BODY" == "Logged out successfully" ]]; then
  echo "PASS: body is exactly 'Logged out successfully'"
else
  echo "FAIL: body is not 'Logged out successfully' (got: '$BODY')"
  cat "$RESPONSE_FILE"
  exit 1
fi

echo "==== MALFORMED REQUEST ===="
# Send a malformed request and expect 400 Bad Request
exec 3<>/dev/tcp/localhost/$PORT
printf "GARBAGE\r\n\r\n" >&3
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
