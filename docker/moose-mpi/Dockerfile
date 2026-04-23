ARG BASE_IMAGE=ghcr.io/idaholab/moose-mpibase/rocky:8.10-gcc13.3.1-mpich5.0.1-openmpi5.0.10_2026.04.12
FROM $BASE_IMAGE

# Remove apptainer if it's installed
RUN (dnf list installed apptainer &> /dev/null && dnf remove -y apptainer) || true

# Extra installs
RUN dnf install -y dnf-plugins-core \
    && dnf config-manager --set-enabled powertools \
    && dnf install -y redhat-lsb-core.x86_64 \
    && dnf install -y epel-release \
    && dnf install -y emacs make cmake diffutils bison flex perl-IO-Compress perl-JSON \
        perl-JSON-PP libtirpc libtirpc-devel zlib-devel patch patchutils libpng \
        libpng-devel valgrind cppunit doxygen fftw-devel gsl-devel libtool autoconf \
        automake cppunit-devel glpk-devel patchelf lcov bash-completion \
    && dnf clean all

