# FunctorTransformFunctorMaterial

!syntax description /FunctorMaterials/FunctorTransformFunctorMaterial

## Overview

This object creates
[functor material properties](Materials/index.md#functor-props), e.g. properties
that get evaluated on-the-fly, as opposed to traditional "static" material
properties, e.g. material properties that are pre-evaluated.
The functor properties are evaluated at the location computed by the evaluation of three functors:

- [!param](/FunctorMaterials/FunctorTransformFunctorMaterial/x_functor) for the x-axis coordinate
- [!param](/FunctorMaterials/FunctorTransformFunctorMaterial/y_functor) for the y-axis coordinate
- [!param](/FunctorMaterials/FunctorTransformFunctorMaterial/z_functor) for the z-axis coordinate

This class is the functor material property (combined) equivalent of
[GenericConstantMaterial.md], [GenericFunctionMaterial.md] and a `variable material`
(material that converts variables to material properties). It evaluates the functor at the requested location,
which can be the element centroid, an element face centroid, a quadrature point,
or any defined overload of the functor argument.

By default this class caches functor evaluations and clears the cache at the beginning
of every time step. Cache clearing behavior can be controlled by setting the `execute_on` parameter.

!alert note
All AD-types of the properties defined in this material must match. Variables are automatically
considered as AD functors, even auxiliary variables. The AD version of this material is `ADFunctorTransformFunctorMaterial`.
Its inputs are a vector of AD functors and it creates AD functor material properties.

!alert warning
This object will throw an error if used in parallel with [!param](/FunctorMaterials/FunctorTransformFunctorMaterial/prop_values)
functors that may require ghosting as this `FunctorMaterial` does not add the proper ghosting
[RelationshipManager](syntax/RelationshipManagers/index.md).

!syntax parameters /FunctorMaterials/FunctorTransformFunctorMaterial

!syntax inputs /FunctorMaterials/FunctorTransformFunctorMaterial

!syntax children /FunctorMaterials/FunctorTransformFunctorMaterial
