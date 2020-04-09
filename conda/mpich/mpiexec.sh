#!/bin/bash
set -euo pipefail
# pipe stdout, stderr through cat to avoid O_NONBLOCK issues
exec mpiexec "$@" 2>&1</dev/null | cat
