## PETSc

Download PETSc 3.8.3

```bash
cd $STACK_SRC
curl -L -O http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-3.8.3.tar.gz
tar -xf petsc-3.8.3.tar.gz -C .
```

Now we configure, build, and install it

```bash
cd $STACK_SRC/petsc-3.8.3

./configure \
--prefix=$PACKAGES_DIR/petsc-3.8.3 \
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
--F77FLAGS='-fPIC -fopenmp'
```

Once configure is done, we build PETSc

```bash
make PETSC_DIR=$STACK_SRC/petsc-3.8.3 PETSC_ARCH=arch-linux2-c-opt all
```

Everything good so far? PETSc should be asking to run more make commands

```bash
make PETSC_DIR=$STACK_SRC/petsc-3.8.3 PETSC_ARCH=arch-linux2-c-opt install
```

And now after the install, we can run some built-in tests

```bash
make PETSC_DIR=$PACKAGES_DIR/petsc-3.8.3 PETSC_ARCH="" test
```

Running the tests should produce some output like the following:

```bash
[moose@centos-7 petsc-3.6.3]$ make PETSC_DIR=$PACKAGES_DIR/petsc-3.8.3 PETSC_ARCH="" test
Running test examples to verify correct installation
Using PETSC_DIR=/opt/MOOSE/petsc-3.8.3 and PETSC_ARCH=
C/C++ example src/snes/examples/tutorials/ex19 run successfully with 1 MPI process
C/C++ example src/snes/examples/tutorials/ex19 run successfully with 2 MPI processes
Fortran example src/snes/examples/tutorials/ex5f run successfully with 1 MPI process
Completed test examples
=========================================
```
