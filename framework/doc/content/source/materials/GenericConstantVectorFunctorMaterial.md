# GenericConstantVectorFunctorMaterial

!syntax description /Materials/GenericConstantVectorFunctorMaterial

The functor version of [GenericConstantVectorFunctorMaterial.md], this can be
used to quickly create simple constant anisotropic material properties, for
testing, for initial survey of a problem or simply because the material
properties do not vary much over the domain explored by the simulation.

## Example Input File Syntax

In this example, we create a `GenericConstantVectorFunctorMaterial` for two
anisotropic friction factors in a porous media flow simulation.  Note the syntax
for declaring two material properties and their values in the same material.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/2d-rc-friction.i block=Materials/darcy

!syntax parameters /Materials/GenericConstantVectorFunctorMaterial

!syntax inputs /Materials/GenericConstantVectorFunctorMaterial

!syntax children /Materials/GenericConstantVectorFunctorMaterial
