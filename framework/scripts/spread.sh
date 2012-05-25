#!/bin/bash

function print_usage
{
 echo -e "Usage: spread.sh <n> <meshfile>\n\tn        - Number of pieces to spread (must be equal to processors)\n\tmeshfile - The Exodus file to spread"
}

if [ $# -lt 2 ]; then
 print_usage;
 exit 1;
fi

base=`basename "$2" .e`
base=`basename "$base" .exd`

ext=${2##*.}

loadbal -inertial -p "$1" -spread -suffix_mesh "$ext" -suffix_spread "$ext" -No_subdirectory "$base"
