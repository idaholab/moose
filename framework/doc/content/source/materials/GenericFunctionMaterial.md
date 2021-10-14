# GenericFunctionMaterial

!syntax description /Materials/GenericFunctionMaterial

This can be used to quickly create simple functionalized material properties, for testing,
for initial survey of a problem or simply because the material properties do not vary much
from the defined function over the domain explored by the simulation.

## Example Input File Syntax

In this example, `GenericFunctionMaterial` is used to define a diffusion
coefficient that is inversely proportional to time. The diffusion coefficient is
retrieved as a `MaterialProperty` by the diffusion kernel.

!listing test/tests/functions/generic_function_material/generic_function_material_test.i block=Materials/gfm

!syntax parameters /Materials/GenericFunctionMaterial

!syntax inputs /Materials/GenericFunctionMaterial

!syntax children /Materials/GenericFunctionMaterial
