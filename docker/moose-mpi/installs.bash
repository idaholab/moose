#!/bin/bash
set -euxo pipefail

# Determine rhel version
IS_RHEL=
if grep -q rhel /etc/os-release; then
    grep 'VERSION=\"8.' /etc/os-release && IS_RHEL=8
    grep 'VERSION=\"9.' /etc/os-release && IS_RHEL=9
else
    echo "Only rhel is supported"
    exit 1
fi

# Remove apptainer if it's installed
if dnf list installed apptainer &> /dev/null; then
    dnf remove -y apptainer
fi

# Enable additional repos
dnf install -y dnf-plugins-core
if [ "$IS_RHEL" == "8" ]; then
    dnf config-manager --set-enabled powertools
    dnf install -y redhat-lsb-core.x86_64
elif [ "$IS_RHEL" == 9 ]; then
    dnf config-manager --set-enabled crb
fi
dnf install -y epel-release

# Additional installs
dnf install -y emacs make cmake diffutils bison flex perl-IO-Compress perl-JSON \
    perl-JSON-PP libtirpc libtirpc-devel zlib-devel patch patchutils libpng \
    libpng-devel valgrind cppunit doxygen fftw-devel gsl-devel libtool autoconf \
    automake cppunit-devel glpk-devel patchelf lcov bash-completion

# Clean after install
dnf clean all
