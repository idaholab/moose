# GenericConstant2DArray

This can be used to quickly create simple constant 2D array material properties, for testing,
for initial survey of a problem or simply because the material properties do not vary much over the
domain explored by the simulation.

## Example Input File Syntax

In this example, `GenericConstant2DArray` is used to define an anisotropic reaction term for this
2D diffusion-reaction simulation. The equation for the second component of $u$ contains both a reaction term
with itself and a source term from the reaction of the first component.

!listing test/tests/bcs/array_vacuum/array_vacuum.i block=Materials/rc

`ADGenericConstant2DArray` can be used to define AD anisotropic reaction coefficients.

!listing test/tests/kernels/array_kernels/ad_array_diffusion_reaction.i block=Materials/rc

!syntax parameters /Materials/GenericConstant2DArray

!syntax inputs /Materials/GenericConstant2DArray

!syntax children /Materials/GenericConstant2DArray
