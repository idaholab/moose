# QuadratureFunctions

The `QuadratureFunctions` syntax is used to declare MFEM coefficients that hold precomputed
values of a source coefficient at the quadrature points of an
[MFEM QuadratureSpace](https://docs.mfem.org/html/classmfem_1_1QuadratureSpace.html), backed by an
[MFEM QuadratureFunction](https://docs.mfem.org/html/classmfem_1_1QuadratureFunction.html). The
declared coefficient may be used in place of the source coefficient wherever a scalar coefficient
name is accepted, avoiding repeated evaluation of expensive source coefficients, and is
re-projected from the source upon a schedule set by the user.
