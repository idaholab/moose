#!/bin/bash

# Supported package managers
MGR_ARY=("apt" "yum")

for MGR in "${MGR_ARY[@]}"; do
    # Run matching install
    if [ $(command -v $MGR | grep -c $MGR) -gt 0 ]; then
        ./${MGR}_installs.sh
    fi
done

# Do pip3 installs
pip3 --no-cache-dir install \
    python-consul \
    python-nomad \
    beautifulsoup4 \
    jinja2 \
    livereload \
    lxml \
    matplotlib \
    mock \
    numpy \
    pandas \
    pybtex \
    pylatexenc \
    pyyaml \
    scikit-image
