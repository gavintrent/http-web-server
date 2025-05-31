#!/usr/bin/env bash
set -e

SERVER_EXEC="../build/bin/server"
PORT=8080

CONFIG_FILE=$(mktemp)
RESPONSE_FILE=$(mktemp)
USER_DB_FILE="../data/users.json"
USERNAME="testuser123654"
PASSWORD="testpass"

# Clean up test artifacts
cleanup() {
  kill $SERVER_PID 2>/dev/null || true
  sleep 0.2
  rm -f "$CONFIG_FILE" "$RESPONSE_FILE"
  if [[ -f "$USER_DB_FILE" ]]; then
    python3 - <<EOF
import json
import os

db_path = os.path.abspath("${USER_DB_FILE}")
username = "${USERNAME}"

try:
    with open(db_path, "r") as f:
        data = json.load(f)
    if username in data:
        del data[username]
        with open(db_path, "w") as f:
            json.dump(data, f, indent=2)
        print(f"Removed user: {username}")
    else:
        print(f"User {username} not found in database.")
except Exception as e:
    print(f"Error cleaning up test user: {e}")
EOF
  fi
}
trap cleanup EXIT

# Create minimal config file for /register
cat > "$CONFIG_FILE" <<EOF
port $PORT;

location /register RegisterHandler {}
EOF

# Start server in background
"$SERVER_EXEC" "$CONFIG_FILE" &
SERVER_PID=$!

sleep 0.2

echo "==== REGISTER CHECK ===="

# Send registration request
curl -s -i -X POST \
  -H "Content-Type: application/json" \
  -d "{\"username\":\"$USERNAME\",\"password\":\"$PASSWORD\"}" \
  "http://localhost:$PORT/register" -o "$RESPONSE_FILE"

# Check 200 OK
if grep -q "HTTP/1.1 200 OK" "$RESPONSE_FILE"; then
  echo "PASS: register returns 200 OK"
else
  echo "FAIL: register did not return 200 OK"
  cat "$RESPONSE_FILE"
  exit 1
fi

# Check Content-Type
if grep -qi "^Content-Type: text/plain" "$RESPONSE_FILE"; then
  echo "PASS: Content-Type is text/plain"
else
  echo "FAIL: missing or wrong Content-Type"
  cat "$RESPONSE_FILE"
  exit 1
fi

# Check response body
BODY=$(awk 'BEGIN {RS="\r\n\r\n"} NR==2 {print}' "$RESPONSE_FILE")
if [[ "$BODY" == "Registration successful" ]]; then
  echo "PASS: correct success body"
else
  echo "FAIL: unexpected body ('$BODY')"
  cat "$RESPONSE_FILE"
  exit 1
fi

echo "==== DUPLICATE USER ===="

# Send same registration again
curl -s -i -X POST \
  -H "Content-Type: application/json" \
  -d "{\"username\":\"$USERNAME\",\"password\":\"$PASSWORD\"}" \
  "http://localhost:$PORT/register" -o "$RESPONSE_FILE"

if grep -q "HTTP/1.1 400 Bad Request" "$RESPONSE_FILE"; then
  echo "PASS: duplicate user returns 400 Bad Request"
else
  echo "FAIL: expected 400 on duplicate registration"
  cat "$RESPONSE_FILE"
  exit 1
fi

echo "==== MALFORMED JSON ===="

curl -s -i -X POST \
  -H "Content-Type: application/json" \
  -d "{\"username\": 12345" \
  "http://localhost:$PORT/register" -o "$RESPONSE_FILE"

if grep -q "HTTP/1.1 400 Bad Request" "$RESPONSE_FILE"; then
  echo "PASS: malformed JSON returns 400 Bad Request"
else
  echo "FAIL: expected 400 for malformed JSON"
  cat "$RESPONSE_FILE"
  exit 1
fi

echo "==== REGISTER INTEGRATION TEST PASSED ===="
exit 0