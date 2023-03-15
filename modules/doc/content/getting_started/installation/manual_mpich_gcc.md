## MPICH

Check and see if you already have an MPI wrapper available on your machine. One simple way of doing
so, is to perform a `which` on the three necessary MPI wrapper binaries:

```bash
which mpicc mpicxx mpif90 | wc -l
```

If the above command returns '3', then you may skip the MPICH section. +However+, if you ended up
building your own GCC compiler above, you will want to *NOT* skip this step. With the reason being,
it is generally best to build an MPI wrapper based on the compiler you plan to use.

Download MPICH [!package!mpich]

!package! code
cd $STACK_SRC
curl -L -O http://www.mpich.org/static/downloads/__MPICH__/mpich-__MPICH__.tar.gz
tar -xf mpich-__MPICH__.tar.gz -C .
!package-end!

Now we create an out-of-tree build location, configure, build, and install it

!package! code max-height=500
mkdir $STACK_SRC/mpich-__MPICH__/gcc-build
cd $STACK_SRC/mpich-__MPICH__/gcc-build

../configure --prefix=$PACKAGES_DIR/mpich-__MPICH__ \
--enable-shared \
--enable-sharedlibs=gcc \
--enable-fast=O2 \
--enable-debuginfo \
--enable-totalview \
--enable-two-level-namespace \
CC=gcc \
CXX=g++ \
FC=gfortran \
F77=gfortran \
F90='' \
CFLAGS='' \
CXXFLAGS='' \
FFLAGS='-fallow-argument-mismatch' \
FCFLAGS='-fallow-argument-mismatch' \
F90FLAGS='' \
F77FLAGS=''

make -j # (where # is the number of cores available)

make install
!package-end!


!alert! note
In order to utilize our newly built MPI wrapper, we need to set some variables:

!package! code
export PATH=$PACKAGES_DIR/mpich-__MPICH__/bin:$PATH
export CC=mpicc
export CXX=mpicxx
export FC=mpif90
export F90=mpif90
export C_INCLUDE_PATH=$PACKAGES_DIR/mpich-__MPICH__/include:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=$PACKAGES_DIR/mpich-__MPICH__/include:$CPLUS_INCLUDE_PATH
export FPATH=$PACKAGES_DIR/mpich-__MPICH__/include:$FPATH
export MANPATH=$PACKAGES_DIR/mpich-__MPICH__/share/man:$MANPATH
export LD_LIBRARY_PATH=$PACKAGES_DIR/mpich-__MPICH__/lib:$LD_LIBRARY_PATH
!package-end!

!alert-end!
