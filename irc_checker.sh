#!/usr/bin/env bash

LOG="$1"

if [[ -z "$LOG" ]]; then
    echo "usage: $0 server.log"
    exit 1
fi

# ---------- colors ----------
RED="\033[1;31m"
GREEN="\033[1;32m"
YELLOW="\033[1;33m"
CYAN="\033[1;36m"
RESET="\033[0m"

awk -v RED="$RED" -v GREEN="$GREEN" -v YELLOW="$YELLOW" -v CYAN="$CYAN" -v RESET="$RESET" '

BEGIN {
    err=0
}

/\[\+\]/ {
    if (match($0, /<([0-9]+)>/, m)) {
        fd=m[1]
        if (state[fd] == 1) {
            printf RED "ERROR: fd %s connected twice without disconnect\n" RESET, fd
            err=1
        }
        state[fd]=1
        conn[fd]++
        total_conn++
    }
}

/\[-\]/ {
    if (match($0, /<([0-9]+)>/, m)) {
        fd=m[1]
        if (state[fd] == 0) {
            printf RED "ERROR: fd %s disconnected without connect\n" RESET, fd
            err=1
        }
        state[fd]=0
        disc[fd]++
        total_disc++
    }
}

END {
    for (fd in state)
        if (state[fd] == 1) {
            printf RED "ERROR: fd %s still connected at end (leak)\n" RESET, fd
            err=1
        }

	print CYAN "Checking Log..." RESET
    print CYAN "Summary" RESET
    print "------------------------"

    printf "connects:     " GREEN "+%d\n" RESET, total_conn
    printf "disconnects:  " GREEN "-%d\n" RESET, total_disc

    if (!err)
        print "\n" GREEN "OK ✓ all lifecycles valid" RESET
    else
        exit 1
}
' "$LOG"