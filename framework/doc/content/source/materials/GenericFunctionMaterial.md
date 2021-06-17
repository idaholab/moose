# GenericFunctionMaterial

!syntax description /Materials/GenericFunctionMaterial

This can be used to quickly create simple functionalized material properties, for testing,
for initial survey of a problem or simply because the material properties do not vary much
from the defined function over the domain explored by the simulation.

## Example Input File Syntax

In this example, `ADGenericFunctionMaterial` is used to define a linearly varying in space
diffusion coefficient for this finite volume diffusion calculation.
We add the prefix `AD` as this simulation is making use of automatic differentiation to compute the Jacobian exactly.
The diffusion coefficient is retrieved as an `ADMaterialProperty` by the diffusion kernel.

!listing test/tests/materials/boundary_material/fv_material_quadrature.i block=Materials/k1

!syntax parameters /Materials/GenericFunctionMaterial

!syntax inputs /Materials/GenericFunctionMaterial

!syntax children /Materials/GenericFunctionMaterial
