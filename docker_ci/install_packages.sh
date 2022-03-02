#!/bin/bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# This script is used in docker_ci/Dockerfile to setup
# the base container environment, and uses
# docker_ci/*_installs.sh based on the distro

# Supported package managers
MGR_ARY=("apt" "yum")

for MGR in "${MGR_ARY[@]}"; do
    # Run matching install
    if [ $(command -v $MGR | grep -c $MGR) -gt 0 ]; then
        ./${MGR}_installs.sh

        if [ $? -ne 0 ]; then
            printf "Error with $MGR package installs.  Aborting build\n"
            exit 1
        fi
    fi
done

# Installing python-pillow on Red Hat distros seems to fail
# due to an old version of pip; so forcefully reinstall it
if [ $(command -v yum | grep -c yum) -gt 0 ]; then
  pip3 --no-cache-dir install -U --force-reinstall pip
fi

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
