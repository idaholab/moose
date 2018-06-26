## MPICH

Download MPICH !!package mpich!!

!package! code
cd $STACK_SRC
curl -L -O http://www.mpich.org/static/downloads/__MPICH__/mpich-__MPICH__.tar.gz
tar -xf mpich-__MPICH__.tar.gz -C .
!package-end!

Now we create an out-of-tree build location, configure, build, and install it

!package! code max-height=500
mkdir $STACK_SRC/mpich-__MPICH__/llvm-build
cd $STACK_SRC/mpich-__MPICH__/llvm-build

../configure --prefix=$PACKAGES_DIR/mpich-__MPICH__ \
--enable-shared \
--enable-sharedlibs=clang \
--enable-fast=03 \
--enable-debuginfo \
--enable-totalview \
--enable-two-level-namespace \
FC=gfortran \
F77=gfortran \
F90='' \
CFLAGS='' \
CXXFLAGS='' \
FFLAGS='' \
FCFLAGS='' \
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
