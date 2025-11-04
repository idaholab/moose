# GenericConstantMaterial / ADGenericConstantMaterial

!syntax description /Materials/GenericConstantMaterial

This can be used to quickly create simple constant material properties, for testing,
for initial survey of a problem or simply because the material properties do not vary much over the
domain explored by the simulation.

## Example Input File Syntax

In this example, we create an `GenericConstantMaterial` for the diffusion coefficient for this fluid flow simulation.

!listing test/tests/auxkernels/diffusion_flux/diffusion_flux.i block=Materials

!syntax parameters /Materials/GenericConstantMaterial

!syntax inputs /Materials/GenericConstantMaterial

!syntax children /Materials/GenericConstantMaterial
