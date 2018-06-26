## PETSc

Download PETSc !!package petsc_default!!

!package code
cd $STACK_SRC
curl -L -O http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-__PETSC_DEFAULT__.tar.gz
tar -xf petsc-__PETSC_DEFAULT__.tar.gz -C .


Now we configure, build, and install it

!package! code max-height=400
cd $STACK_SRC/petsc-__PETSC_DEFAULT__

./configure \
--prefix=$PACKAGES_DIR/petsc-__PETSC_DEFAULT__ \
--download-hypre=1 \
--with-ssl=0 \
--with-debugging=no \
--with-pic=1 \
--with-shared-libraries=1 \
--with-cc=mpicc \
--with-cxx=mpicxx \
--with-fc=mpif90 \
--download-fblaslapack=1 \
--download-metis=1 \
--download-parmetis=1 \
--download-superlu_dist=1 \
--download-mumps=1 \
--download-scalapack=1 \
--CC=mpicc --CXX=mpicxx --FC=mpif90 --F77=mpif77 --F90=mpif90 \
--CFLAGS='-fPIC -fopenmp' \
--CXXFLAGS='-fPIC -fopenmp' \
--FFLAGS='-fPIC -fopenmp' \
--FCFLAGS='-fPIC -fopenmp' \
--F90FLAGS='-fPIC -fopenmp' \
--F77FLAGS='-fPIC -fopenmp' \
PETSC_DIR=`pwd`
!package-end!

Once configure is done, we build PETSc

!package code
make PETSC_DIR=$STACK_SRC/petsc-__PETSC_DEFAULT__ PETSC_ARCH=arch-linux2-c-opt all

Everything good so far? PETSc should be asking to run more make commands

!package code
make PETSC_DIR=$STACK_SRC/petsc-__PETSC_DEFAULT__ PETSC_ARCH=arch-linux2-c-opt install

And now after the install, we can run some built-in tests

!package code
make PETSC_DIR=$PACKAGES_DIR/petsc-__PETSC_DEFAULT__ PETSC_ARCH="" test

Running the tests should produce some output like the following:

!package code
[moose@centos-7 petsc-__PETSC_DEFAULT__]$ make PETSC_DIR=$PACKAGES_DIR/petsc-__PETSC_DEFAULT__ PETSC_ARCH="" test
Running test examples to verify correct installation
Using PETSC_DIR=/opt/moose/petsc-__PETSC_DEFAULT__ and PETSC_ARCH=
C/C++ example src/snes/examples/tutorials/ex19 run successfully with 1 MPI process
C/C++ example src/snes/examples/tutorials/ex19 run successfully with 2 MPI processes
Fortran example src/snes/examples/tutorials/ex5f run successfully with 1 MPI process
Completed test examples
=========================================
