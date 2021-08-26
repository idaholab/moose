# GenericFunctionFunctorMaterial

!syntax description /Materials/GenericFunctionFunctorMaterial

## Overview

This class template is the functor material property version of
[GenericFunctionMaterial.md]. By default this class caches function evaluations
and clears the cache at the beginning of every timestep. Cach clearing behavior can be
controlled by setting the `execute_on` parameter.

## Example Input File Syntax

In this example, `ADGenericFunctionMaterial` is used to define a linearly varying in space
diffusion coefficient for this finite volume diffusion calculation.
We add the prefix `AD` as this simulation is making use of automatic differentiation to compute the Jacobian exactly.
The diffusion coefficient is retrieved as a `FunctorInterface<ADReal>`, the base class
of `FunctorMaterialProperty<ADReal>`, by the diffusion kernel.

!listing test/tests/materials/boundary_material/fv_material_quadrature.i block=Materials/k1

!syntax parameters /Materials/GenericFunctionFunctorMaterial

!syntax inputs /Materials/GenericFunctionFunctorMaterial

!syntax children /Materials/GenericFunctionFunctorMaterial
