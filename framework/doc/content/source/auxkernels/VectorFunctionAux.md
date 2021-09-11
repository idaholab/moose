# VectorFunctionAux

!syntax description /AuxKernels/VectorFunctionAux

The `VectorFunctionAux` helps turn a vector field defined by a function into a vector variable. It is generally
helpful when the spatial and temporal dependence of a field is known ahead of time, and the kernels
and other objects needing that field expect a vector variable rather than a function.

## Example syntax

In this example, the `VectorFunctionAux` is used to set the vector variable `vec` to the
values computed by the aptly named `ParsedVectorFunction` `function`.

!listing test/tests/auxkernels/vector_function_aux/vector_function_aux.i block=Functions AuxKernels

!syntax parameters /AuxKernels/VectorFunctionAux

!syntax inputs /AuxKernels/VectorFunctionAux

!syntax children /AuxKernels/VectorFunctionAux
