# GenericConstantVectorMaterial

!syntax description /Materials/GenericConstantVectorMaterial

This can be used to quickly create simple constant anisotropic material properties, for testing,
for initial survey of a problem or simply because the material properties do not vary much over the
domain explored by the simulation.

## Example Input File Syntax

In this example, we create a `GenericConstantVectorMaterial` for two anisotropic friction factors in a porous media flow simulation.
Note the syntax for declaring two material properties and their values in the same material.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc-friction.i block=Materials/darcy

!syntax parameters /Materials/GenericConstantVectorMaterial

!syntax inputs /Materials/GenericConstantVectorMaterial

!syntax children /Materials/GenericConstantVectorMaterial
