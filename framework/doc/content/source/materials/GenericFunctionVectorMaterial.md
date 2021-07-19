# GenericFunctionVectorMaterial

!syntax description /Materials/GenericFunctionVectorMaterial

This can be used to quickly create simple functionalized vector material properties, for testing,
for initial survey of a problem or simply because the material properties do not vary much
from the defined function over the domain explored by the simulation.

## Example Input File Syntax

In this example, `ADGenericFunctionVectorMaterial` is used to define a linearly varying in space,
quadratic in time anisotropic diffusion coefficient for this finite volume diffusion calculation.
We add the prefix `AD` as this simulation is making use of automatic differentiation to compute the Jacobian exactly.
The diffusion coefficient is retrieved as an `ADMaterialProperty` by the diffusion kernel.

!listing test/tests/functions/generic_function_material/generic_function_vector_material_test.i block=Functions Materials/gfm

!syntax parameters /Materials/GenericFunctionVectorMaterial

!syntax inputs /Materials/GenericFunctionVectorMaterial

!syntax children /Materials/GenericFunctionVectorMaterial
