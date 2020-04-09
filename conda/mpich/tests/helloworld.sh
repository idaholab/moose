#!/bin/bash
set -eu
rank=$PMI_RANK
size=$PMI_SIZE
printf "Hello, World! I am process %d of %d.\n" $rank $size
