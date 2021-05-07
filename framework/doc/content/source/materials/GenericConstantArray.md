# GenericConstantArray

!syntax description /Materials/GenericConstantArray

Array material properties are multi-dimensional material properties. Note that vector material properties
are also an option if the array dimension is really the number of dimensions of the problem (1D/2D/3D).

This can be used to quickly create simple constant 1D array material properties, for testing,
for initial survey of a problem or simply because the material properties do not vary much over the
domain explored by the simulation.

## Example Input File Syntax

In this example, `GenericConstantArray` is used to define an anisotropic diffusion coefficient for this 2D anisotropic diffusion-reaction
problem.

!listing test/tests/bcs/array_vacuum/array_vacuum.i block=Materials/dc

!syntax parameters /Materials/GenericConstantArray

!syntax inputs /Materials/GenericConstantArray

!syntax children /Materials/GenericConstantArray
