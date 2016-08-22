## Description
The element "size" is often needed for creating stabilization coefficients or when adapting the mesh. This will compute the minimum or maximum element size and populate an [AuxVariable](auto::/AuxVariables/Overview)
with the result. The element size calculation uses the [libMesh](http://libmesh.github.io/) `hmin()` or `hmax()` method
from the [`Elem`](https://libmesh.github.io/doxygen/classlibMesh_1_1Elem.html) class to compute the size.

## Example Syntax
![](test/tests/auxkernels/element_size/element_size.i::AuxKernels)
