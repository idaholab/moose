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
# the base container environment for Red Hat-based images

# Want to exit if there's an errror
set -e

# Only want to mess with mirrors for CentOS
if [ $(ls /etc/yum.repos.d | grep -c CentOS) -gt 0 ]; then
  # Per https://stackoverflow.com/a/70930049, we need these commands for Yum mirrors
  # Per https://serverfault.com/a/1093928, switch to vault.epel.cloud
  sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/CentOS-Linux-*
  sed -i 's|#baseurl=http://mirror.centos.org|baseurl=http://vault.epel.cloud|g' /etc/yum.repos.d/CentOS-Linux-*
fi

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
  gtest \
  sudo \
  file \
  zlib-devel \
  rsync \
  cmake

# Clear cache
yum clean all
rm -rf /var/cache/yum/

# Make symbolic link for invoking Python
ln -s /usr/bin/python3 /usr/bin/python
