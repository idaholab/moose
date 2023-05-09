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
# the base container environment for Debian-based images

# Want to exit if there's an errror
set -e

export DEBIAN_FRONTEND=noninteractive

# Update package lists
apt-get update -y

# Install packages
apt-get install -y \
  build-essential \
  gfortran \
  tcl \
  git \
  m4 \
  freeglut3 \
  doxygen \
  libblas-dev \
  liblapack-dev \
  libx11-dev \
  libnuma-dev \
  libcurl4-gnutls-dev \
  zlib1g-dev \
  libhwloc-dev \
  python3 \
  python3-dev \
  python3-pip \
  curl \
  bison \
  flex \
  libboost-all-dev \
  emacs \
  libgtest-dev \
  sudo \
  update-motd \
  rsync \
  cmake

# Clear cache
rm -rf /var/lib/apt/lists/*

# Set Python 3 as the default interpreter
update-alternatives --install /usr/bin/python python /usr/bin/python3 10
