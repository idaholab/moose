#!/bin/bash

# Enable powertools if present
PT_REPO=/etc/yum.repos.d/CentOS-PowerTools.repo
if [ -f $PT_REPO ]; then
  sed -i 's|^enabled=0|enabled=1|g' $PT_REPO
fi

# Needed for subsequent yum installs to work
touch /var/lib/rpm/*

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
  emacs

# Clear cache
yum clean all
rm -rf /var/cache/yum/

# Make symbolic link for invoking Python
ln -s /usr/bin/python3 /usr/bin/python
