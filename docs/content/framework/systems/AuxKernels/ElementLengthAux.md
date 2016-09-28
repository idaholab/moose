!devel /Kernels/Diffusion float=right width=auto margin=20px padding=20px background-color=#F8F8F8

# ElementLengthAux
!description /AuxKernels/ElementLengthAux

## Description
The element "length" is often needed for creating stabilization coefficients or when adapting the mesh. This will compute the minimum or maximum element length and populate an [AuxVariable](/AuxVariables/Overview.md)
with the result. The element size calculation uses the [libMesh](http://libmesh.github.io/) `hmin()` or `hmax()` method
from the [`Elem`](https://libmesh.github.io/doxygen/classlibMesh_1_1Elem.html) class to compute the length.

## Example Syntax
![](test/tests/auxkernels/element_length/element_length.i::AuxKernels)

!parameters /AuxKernels/ElementLengthAux

!inputfiles /AuxKernels/ElementLengthAux

!childobjects /AuxKernels/ElementLengthAux
