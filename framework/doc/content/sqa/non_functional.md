# Use, Performance, and System Interfaces for MOOSE

## Usability

MOOSE-based applications are designed to be operated from the command-line as a single binary
application. This binary when executed will produce a list of available options if executed without
any arguments. In practice, the binary expects and input file for defining the simulation to perform.

```
./moose_test-opt -i input.i
```

The command-line interface includes the ability to add or override parameters within the
supplied input file from the command-line. This is accomplished by adding the parameter to
be modified (using the full syntax) at the end of the command.

```
./moose_test-opt -i input.i Mesh/uniform_refine=1
```

If an invalid input to the command-line is provided the binary application will produce an
error message to aid in correcting the mistake. And during simulations the binary will begin
by displaying information regarding the current application, dependencies, and current execution.

## Performance

MOOSE-based applications are designed to operate in parallel with distributed and/or shared memory
parallel operation on modern macOS and Linux operating systems. Distributed memory operation
uses the [!ac](MPI) and shared memory operation may use a variety of models such as
[OpenMP](https://www.openmp.org) or [pthreads](https://computing.llnl.gov/tutorials/pthreads).

## System Interfaces

MOOSE-based applications may be compiled with modern [Clang](https://clang.llvm.org) or
[GCC](https://gcc.gnu.org) C++ compilers. The application also supports optional packages via the
libMesh and PETSc configuration. Examples include the [MUMPS](http://mumps.enseeiht.fr) and
[SuperLU](https://portal.nersc.gov/project/sparse/superlu/) solver packages. Please refer to the
documentation for libMesh and PETSc for further details.
