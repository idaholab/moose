# GenericFunctorMaterial

!syntax description /Materials/GenericFunctorMaterial

## Overview

This object creates
[functor material properties](Materials/index.md#functor-props), e.g. properties
that get evaluated on-the-fly, as opposed to traditional "static" material
properties, e.g. material properties that are pre-evaluated.

This class is the functor material property (combined) equivalent of
[GenericConstantMaterial.md], [GenericFunctionMaterial.md] and a `variable material`
(material that converts variables to material properties). It evaluates the functor at the requested location,
which can be the element centroid, an element face centroid, a quadrature point,
or any defined overload of the functor argument.

By default this class caches functor evaluations and clears the cache at the beginning
of every time step. Cache clearing behavior can be controlled by setting the `execute_on` parameter.

!alert note
All AD-types of the properties defined in this material must match. Variables are automatically
considered as AD functors, even auxiliary variables. The AD version of this material is `ADGenericFunctorMaterial`.
Its inputs are a vector of AD functors and it creates AD functor material properties.

## Example Input File Syntax

In this example, `ADGenericFunctorMaterial` is used to define a linearly varying in space
diffusion coefficient for this finite volume diffusion calculation.
We add the prefix `AD` as this simulation is making use of automatic differentiation to compute the Jacobian exactly.
The diffusion coefficient is retrieved as a `Moose::Functor<ADReal>`, the base class
of `FunctorMaterialProperty<ADReal>`, by the diffusion kernel. The diffusion kernel can
then obtain the diffusion coefficient directly on the faces when evaluating the face flux.

!listing test/tests/materials/boundary_material/fv_material_quadrature.i block=Materials/k1

!syntax parameters /Materials/GenericFunctorMaterial

!syntax inputs /Materials/GenericFunctorMaterial

!syntax children /Materials/GenericFunctorMaterial
