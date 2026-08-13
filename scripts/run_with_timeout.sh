#!/bin/bash
# Run a command, killing it if it outlives a deadline. Stand-in for GNU
# `timeout`, which macOS bash does not ship by default.
#
# Usage: run_with_timeout.sh <seconds> <command> [args...]

set -u

seconds="$1"
shift

"$@" &
pid=$!

for _ in $(seq 1 "$seconds"); do
  kill -0 "$pid" 2>/dev/null || exit 0
  sleep 1
done

if kill -0 "$pid" 2>/dev/null; then
  echo "run_with_timeout: killing after ${seconds}s: $*" >&2
  kill -9 "$pid" 2>/dev/null
  wait "$pid" 2>/dev/null
  exit 124
fi
