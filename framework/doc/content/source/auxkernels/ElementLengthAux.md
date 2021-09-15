# ElementLengthAux

!syntax description /AuxKernels/ElementLengthAux

## Description

The element "length" is often needed for creating stabilization coefficients or when adapting the mesh. This will compute the minimum or maximum element length and populate an [AuxVariable](/AuxVariables/index.md)
with the result. The element size calculation uses the [libMesh] `hmin()` or `hmax()` method
from the [`Elem`](https://libmesh.github.io/doxygen/classlibMesh_1_1Elem.html) class to compute the length.

## Example Syntax

!listing test/tests/auxkernels/element_length/element_length.i block=AuxKernels

!syntax parameters /AuxKernels/ElementLengthAux

!syntax inputs /AuxKernels/ElementLengthAux

!syntax children /AuxKernels/ElementLengthAux
