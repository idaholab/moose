# VectorVariableMagnitudeAux

The `VectorVariableMagnitudeAux` class takes a vector variable, specified
through the `vector_variable` parameter, and generates an auxiliary variable
corresponding to the Euclidean norm of the vector variable's components.
This object is
only meant to be used with `LAGRANGE_VEC` vector variables, and hence the
auxiliary variable should be of type `LAGRANGE`.

## Example Inpute Syntax

!listing test/tests/auxkernels/vector_variable_nodal/vector_variable_nodal.i block=AuxKernels

!syntax description /AuxKernels/VectorVariableMagnitudeAux

!syntax parameters /AuxKernels/VectorVariableMagnitudeAux

!syntax inputs /AuxKernels/VectorVariableMagnitudeAux

!syntax children /AuxKernels/VectorVariableMagnitudeAux
