## MPICH

Download MPICH 3.2

```bash
cd $STACK_SRC
curl -L -O http://www.mpich.org/static/downloads/3.2/mpich-3.2.tar.gz
tar -xf mpich-3.2.tar.gz -C .
```

Now we create an out-of-tree build location, configure, build, and install it

```bash
mkdir $STACK_SRC/mpich-3.2/llvm-build
cd $STACK_SRC/mpich-3.2/llvm-build

../configure --prefix=$PACKAGES_DIR/mpich-3.2 \
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
```

!alert! note
In order to utilize our newly built MPI wrapper, we need to set some variables:

```bash
export PATH=$PACKAGES_DIR/mpich-3.2/bin:$PATH
export CC=mpicc
export CXX=mpicxx
export FC=mpif90
export F90=mpif90
export C_INCLUDE_PATH=$PACKAGES_DIR/mpich-3.2/include:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=$PACKAGES_DIR/mpich-3.2/include:$CPLUS_INCLUDE_PATH
export FPATH=$PACKAGES_DIR/mpich-3.2/include:$FPATH
export MANPATH=$PACKAGES_DIR/mpich-3.2/share/man:$MANPATH
export LD_LIBRARY_PATH=$PACKAGES_DIR/mpich-3.2/lib:$LD_LIBRARY_PATH
```
!alert-end!
