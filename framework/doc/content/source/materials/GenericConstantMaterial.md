# GenericConstantMaterial

!syntax description /Materials/GenericConstantMaterial

This can be used to quickly create simple constant material properties, for testing,
for initial survey of a problem or simply because the material properties do not vary much over the
domain explored by the simulation.

## Example Input File Syntax

In this example, we create an `ADGenericConstantMaterial` for the diffusion coefficient for this fluid flow simulation.
We add the prefix `AD` as this simulation is making use of automatic differentiation to compute the Jacobian exactly.
The diffusion coefficient is retrieved as an `ADMaterialProperty` by the diffusion kernel.

!listing test/tests/fvkernels/mms/advection-diffusion.i block=Materials/diff

!syntax parameters /Materials/GenericConstantMaterial

!syntax inputs /Materials/GenericConstantMaterial

!syntax children /Materials/GenericConstantMaterial
