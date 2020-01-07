#!/bin/bash

# Update package lists
yum update -y

# Install packages
yum install -y \
  gcc \
  gcc-c++ \
  gcc-gfortran \
  git \
  make \
  cmake \
  tcl \
  m4 \
  freeglut-devel \
  blas-devel \
  lapack-devel \
  libX11-devel \
  python-devel \
  wget \
  which \
  boost-devel \
  bison \
  flex \
  tar

# Clear cache
yum clean all
rm -rf /var/cache/yum/
