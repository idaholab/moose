#!/bin/bash
# Build the modules/contact unit test binary in a named conda environment and
# optionally run it (under mpiexec, with a hard timeout via run_with_timeout.sh).
#
# Usage:
#   scripts/contact_unit_build_and_test.sh --conda-env ENV [options]
#
# Options:
#   --conda-env ENV     conda environment to activate (required)
#   --jobs N             parallel build jobs (default: 4)
#   --ranks N             mpiexec rank count for the run step (default: 1)
#   --filter PATTERN      gtest --gtest_filter pattern (default: run everything)
#   --timeout SECONDS     kill the run after this many seconds (default: 120)
#   --build-only          only build, skip running the tests
#   --run-only            only run the tests, skip building

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
UNIT_DIR="$REPO_ROOT/modules/contact/unit"

conda_env=""
jobs=4
ranks=1
filter=""
timeout_seconds=120
do_build=1
do_run=1

while [[ $# -gt 0 ]]; do
  case "$1" in
    --conda-env) conda_env="$2"; shift 2 ;;
    --jobs) jobs="$2"; shift 2 ;;
    --ranks) ranks="$2"; shift 2 ;;
    --filter) filter="$2"; shift 2 ;;
    --timeout) timeout_seconds="$2"; shift 2 ;;
    --build-only) do_run=0; shift ;;
    --run-only) do_build=0; shift ;;
    *) echo "Unknown argument: $1" >&2; exit 1 ;;
  esac
done

if [[ -z "$conda_env" ]]; then
  echo "error: --conda-env is required" >&2
  exit 1
fi

# Conda's own activation scripts (e.g. gfortran_osx-arm64) reference unset
# variables, which trips our `set -u`; relax it just for activation.
set +u
# shellcheck disable=SC1091
source "$(conda info --base)/etc/profile.d/conda.sh"
conda activate "$conda_env"
set -u

if [[ "$do_build" == 1 ]]; then
  make -C "$UNIT_DIR" -j"$jobs"
  build_status=$?
  if [[ $build_status -ne 0 ]]; then
    exit $build_status
  fi
fi

if [[ "$do_run" == 1 ]]; then
  gtest_args=()
  if [[ -n "$filter" ]]; then
    gtest_args+=("--gtest_filter=$filter")
  fi
  "$SCRIPT_DIR/run_with_timeout.sh" "$timeout_seconds" \
    mpiexec -n "$ranks" "$UNIT_DIR/contact-unit-opt" "${gtest_args[@]+"${gtest_args[@]}"}"
  exit $?
fi
