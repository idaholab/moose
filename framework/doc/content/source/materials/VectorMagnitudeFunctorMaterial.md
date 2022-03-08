# VectorMagnitudeFunctorMaterial

!syntax description /Materials/VectorMagnitudeFunctorMaterial

## Overview

This class either takes 1-3 scalar-valued (e.g. `Real`, `ADReal`) functors or a
single vector functor and creates a functor that returns the Euclidean norm of
the input. For creating a `Real`-returning functor that takes in `Real` input,
use the `VectorMagnitudeFunctorMaterial` type in the input file. For a
`ADReal`-returning functor that takes in `ADReal` input, use
`ADVectorMagnitudeFunctorMaterial`. If using component inputs and the y or
z-component functor parameters are not provided, then they are defaulted to 0.

## Example Input File Syntax

### Scalar-component inputs

In this example, `ADVectorMagnitudeFunctorMaterial` is used to define the vector
magnitude of the vector component inputs `u` and `v` which happen to be
nonlinear variables in this case. `u` varies from 0 to 1 from bottom to top and
`v` varies from 0 to 1 from left to right, such that the magnitude field is
symmetric about the line y=x with the field value increasing moving to the top-right.

!listing test/tests/materials/functor_properties/vector-magnitude/test.i block=Materials/functor

### Vector input

In this example, the functor provided by `ADVectorMagnitudeFunctorMaterial`
(which we name `mat_mag` to avoid collision with the auxiliary variable `mag`)
computes the norm of the vector functor `u`, which is a nonlinear variable in
this simulation.

!listing test/tests/materials/functor_properties/vector-magnitude/vector-test.i block=Materials/functor

!syntax parameters /Materials/VectorMagnitudeFunctorMaterial

!syntax inputs /Materials/VectorMagnitudeFunctorMaterial

!syntax children /Materials/VectorMagnitudeFunctorMaterial
