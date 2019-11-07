#!/bin/bash

# Supported package managers
MGR_ARY=("apt" "yum")

for MGR in "${MGR_ARY[@]}"; do
    # Run matching install
    if [ $(command -v $MGR | grep -c $MGR) -gt 0 ]; then
        ./${MGR}_installs.sh
    fi
done
