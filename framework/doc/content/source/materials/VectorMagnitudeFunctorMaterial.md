# VectorMagnitudeFunctorMaterial

!syntax description /Materials/VectorMagnitudeFunctorMaterial

## Overview

For creating a `Real`-returning functor that takes in `Real` component inputs,
use the `VectorMagnitudeFunctorMaterial` type in the input file. For a
`ADReal`-returning functor that takes in `ADReal` component inputs, use
`ADVectorMagnitudeFunctorMaterial`. If a component functor parameter is not provided, then
its value is defaulted to 0, with the exception of the `x_functor` parameter, which is required.

## Example Input File Syntax

In this example, `ADVectorMagnitudeFunctorMaterial` is used to define the vector
magnitude of the vector component inputs `u` and `v` which happen to be
nonlinear variables in this case. `u` varies from 0 to 1 from bottom to top and
`v` varies from 0 to 1 from left to right, such that the magnitude field is
symmetric about the line y=x with the field value increasing moving to the top-right.

!listing test/tests/materials/functor_properties/vector-magnitude/test.i block=Materials/functor

!syntax parameters /Materials/VectorMagnitudeFunctorMaterial

!syntax inputs /Materials/VectorMagnitudeFunctorMaterial

!syntax children /Materials/VectorMagnitudeFunctorMaterial
