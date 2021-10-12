# GenericFunctionFunctorMaterial

!syntax description /Materials/GenericFunctionFunctorMaterial

## Overview

This class template is the functor material property version of
[GenericFunctionMaterial.md]. It evaluates the function at the requested location,
which can be the element centroid, an element face centroid, a quadrature point,
or any defined overload of the functor argument.

By default this class caches function evaluations
and clears the cache at the beginning of every timestep. Cache clearing behavior can be
controlled by setting the `execute_on` parameter.

## Example Input File Syntax

In this example, `ADGenericFunctionMaterial` is used to define a linearly varying in space
diffusion coefficient for this finite volume diffusion calculation.
We add the prefix `AD` as this simulation is making use of automatic differentiation to compute the Jacobian exactly.
The diffusion coefficient is retrieved as a `Moose::Functor<ADReal>`, the base class
of `FunctorMaterialProperty<ADReal>`, by the diffusion kernel. The diffusion kernel can
then obtain the diffusion coefficient directly on the faces when evaluating the face flux.

!listing test/tests/materials/boundary_material/fv_material_quadrature.i block=Materials/k1

!syntax parameters /Materials/GenericFunctionFunctorMaterial

!syntax inputs /Materials/GenericFunctionFunctorMaterial

!syntax children /Materials/GenericFunctionFunctorMaterial
