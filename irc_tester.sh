#!/usr/bin/env bash

# ==================================================
# Colors
# ==================================================
RED="\033[1;31m"
GREEN="\033[1;32m"
CYAN="\033[1;36m"
RESET="\033[0m"

# ==================================================
# Ctrl-C handler
# ==================================================
cleanup() {
    echo -e "\n${RED}Stopped by user. Exiting...${RESET}"
    exit 0
}
trap cleanup INT

# ==================================================
# Usage
# ==================================================
usage() {
    echo "Usage:"
    echo "  $0 <host> <port>"
    exit 1
}

check_server() {
    echo -ne "${CYAN}Checking server availability... ${RESET}"

    if nc -z -w 1 "$HOST" "$PORT" 2>/dev/null; then
        echo -e "${GREEN}OK${RESET}"
    else
        echo -e "${RED}FAILED${RESET}"
        echo -e "${RED}Server not listening on $HOST:$PORT${RESET}"
        exit 1
    fi
}

# ==================================================
# Argument checking
# ==================================================
[[ "$1" == "-h" || "$1" == "--help" ]] && usage

if [[ $# -ne 2 ]]; then
    echo -e "${RED}Error: missing arguments${RESET}"
    usage
fi

HOST="$1"
PORT="$2"

if ! [[ "$PORT" =~ ^[0-9]+$ ]] || (( PORT < 1 || PORT > 65535 )); then
    echo -e "${RED}Error: invalid port '$PORT'${RESET}"
    exit 1
fi

print_test() {
    echo -e "\n${CYAN}=== Testing: $1 ===${RESET}"
}

ok() {
    echo -e "${GREEN}✓ done${RESET}"
}

# ==================================================
# Tests (unchanged)
# ==================================================
test_no_crlf_overflow() {
    print_test "no CRLF overflow (20KB raw stream)"

    python3 - <<EOF | nc -q 0 "$HOST" "$PORT"
import sys
sys.stdout.write("A" * 20000)
EOF

    ok
}

test_split_large_line() {
    print_test "split large line (300 + 300 + CRLF)"

    (
      printf "A%.0s" {1..300}
      sleep 1
      printf "A%.0s" {1..300}
      printf "\r\n"
    ) | nc -q 0 "$HOST" "$PORT"

    ok
}

test_many_big_commands() {
    print_test "50 concurrent oversized commands (>512)"

    python3 - <<EOF | nc -q 0 "$HOST" "$PORT" &
import sys
sys.stdout.write("A"*600 + "\r\n")
EOF

    for i in {2..50}; do
      python3 - <<EOF | nc -q 0 "$HOST" "$PORT" >/dev/null 2>&1 &
import sys
sys.stdout.write("A"*600 + "\r\n")
EOF
    done

    wait
    ok
}

test_normal_clients() {
    print_test "15 normal clients (Ping)"

    (printf "Ping\r\n") | nc -q 0 "$HOST" "$PORT" &

    for i in {2..15}; do
      (printf "Ping\r\n") | nc -q 0 "$HOST" "$PORT" >/dev/null 2>&1 &
    done

    wait
    ok
}

# ==================================================
# Run continuously
# ==================================================
echo -e "${CYAN}IRC Server Stress Tester${RESET}"
echo "Target: $HOST:$PORT"
echo "Press CTRL-C to stop"

check_server

ITER=1
while true; do
    echo -e "\n${CYAN}===== Loop $ITER =====${RESET}"

    test_no_crlf_overflow
    test_split_large_line
    test_many_big_commands
    test_normal_clients

    ((ITER++))
    sleep 1   # small pause so it doesn't hammer 100% CPU
done