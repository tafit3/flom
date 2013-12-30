#!/bin/sh
# use this trivial script to check memory behavior
export G_SLICE=always-malloc
valgrind --leak-check=full src/flom -t /tmp/valgrind-daemon.trace -T /tmp/valgrind-command.trace -- ls >/tmp/stdout 2>/tmp/stderr
