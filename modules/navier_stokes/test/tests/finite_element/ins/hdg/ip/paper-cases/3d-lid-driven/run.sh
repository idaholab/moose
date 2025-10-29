#!/bin/bash

# Base executable and input file
EXEC=/data/lindad/projects/moose4/modules/navier_stokes/navier_stokes-opt
INPUT=lid-driven-strumpack.i

# Starting parameters
procs=1
refine=0

# Number of runs
num_cases=3

for ((case=1; case<=num_cases; case++)); do
    # Build identifiers
    tag="strumpack-${procs}proc-${refine}refine"

    echo "=== Running case ${case}: ${procs} ranks, refine=${refine} ==="

    # Run the case
    mpiexec -np "$procs" "$EXEC" -i "$INPUT" Mesh/uniform_refine="$refine" Outputs/file_base="${tag}" --color off \
        2>&1 | tee "${tag}.log"

    echo "=== Finished case ${case} ==="
    echo

    # Update parameters
    procs=$((procs * 8))
    refine=$((refine + 1))
done
