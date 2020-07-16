#!/bin/bash

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
  cmake \
  curl \
  bison \
  flex \
  libboost-all-dev \
  emacs

# Clear cache
rm -rf /var/lib/apt/lists/*

# Set Python 3 as the default interpreter
update-alternatives --install /usr/bin/python python /usr/bin/python3 10
