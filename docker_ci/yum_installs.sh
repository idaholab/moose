#!/bin/bash

# Update package lists
yum update -y

# Enable powertools repo
yum install -y dnf-plugins-core
yum config-manager --set-enabled powertools

# Needed for subsequent yum installs to work
touch /var/lib/rpm/*

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
  python3-devel \
  python3 \
  diffutils \
  wget \
  which \
  boost-devel \
  bison \
  flex \
  tar \
  libtool \
  libtirpc \
  libtirpc-devel \
  emacs \
  gtest

# Clear cache
yum clean all
rm -rf /var/cache/yum/

# Make symbolic link for invoking Python
ln -s /usr/bin/python3 /usr/bin/python
