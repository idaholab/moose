# GenericFunctorGradientMaterial

!syntax description /FunctorMaterials/GenericFunctorGradientMaterial

## Overview

This object creates
[vector functor material properties](Materials/index.md#functor-props) that are gradients of
other functors.

!alert note
All AD-types of the properties defined in this material must match. Variables are automatically
considered as AD functors, even auxiliary variables. The AD version of this material is `ADGenericFunctorGradientMaterial`.
Its inputs are a vector of AD functors and it creates AD vector functor material properties.

## Example Input File Syntax

In this example, `ADGenericFunctorGradientMaterial` is used to compute the gradient of
the variable `u`, which is then multiplied by a diffusion coefficient to obtain the diffusive
flux. This flux is computed at the element centroids, not one the element faces.

!listing test/tests/materials/functor_properties/gradients/functor-gradients.i block=Materials AuxKernels

!syntax parameters /FunctorMaterials/GenericFunctorGradientMaterial

!syntax inputs /FunctorMaterials/GenericFunctorGradientMaterial

!syntax children /FunctorMaterials/GenericFunctorGradientMaterial
