```bash
./configure \
--prefix=$PETSC_DIR \
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
--download-scalapack=1 \
--download-mumps=1 \
CC=mpicc CXX=mpicxx FC=mpif90 F77=mpif77 F90=mpif90 \
CFLAGS='-fPIC -fopenmp' \
CXXFLAGS='-fPIC -fopenmp' \
FFLAGS='-fPIC -fopenmp' \
FCFLAGS='-fPIC -fopenmp' \
F90FLAGS='-fPIC -fopenmp' \
F77FLAGS='-fPIC -fopenmp' \
PETSC_DIR=`pwd`
```

!alert note
The options used were chosen exclusively for GCC compilers. If you plan on using an Intel compiler,
know that these options will not work (-fopenmp needs to be -openmp for one thing. fblaslapack may
also fail as Intel provides its own).
