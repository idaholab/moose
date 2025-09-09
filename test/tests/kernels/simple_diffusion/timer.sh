#!/bin/bash

# Path to the script you want to run
SCRIPT=".././../../moose_test-opt -i simple_diffusion.i Outputs/exodus=false"

# Number of times to run the script
RUNS=20

# Arrays to store durations
durations=()

echo "Running $SCRIPT $RUNS times..."

for i in $(seq 1 $RUNS); do
    echo "Run #$i..."
    start_time=$(date +%s.%N)
    $SCRIPT
    end_time=$(date +%s.%N)

    # Calculate duration
    duration=$(echo "$end_time - $start_time" | bc)
    durations+=($duration)
    echo "Duration: $duration seconds"
done

# Calculate average
total_time=0
for d in "${durations[@]}"; do
    total_time=$(echo "$total_time + $d" | bc)
done
average=$(echo "scale=6; $total_time / $RUNS" | bc)

# Calculate standard deviation
sum_sq_diff=0
for d in "${durations[@]}"; do
    diff=$(echo "$d - $average" | bc)
    sq_diff=$(echo "$diff * $diff" | bc)
    sum_sq_diff=$(echo "$sum_sq_diff + $sq_diff" | bc)
done
stddev=$(echo "scale=6; sqrt($sum_sq_diff / $RUNS)" | bc -l)

echo ""
echo "Average runtime: $average seconds"
echo "Standard deviation: $stddev seconds"
