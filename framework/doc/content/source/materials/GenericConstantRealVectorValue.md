# GenericConstantRealVectorValue

!syntax description /Materials/GenericConstantRealVectorValue

## Overview

`GenericConstantRealVectorValue` creates a `RealVectorValue` material property that use
constant values to fill the value.

This can be used to quickly create simple constant 3-vector properties, for testing,
for initial survey of a problem or simply because the material properties do not vary much over the
domain explored by the simulation.

The AD counterpart `ADGenericConstantRealVectorValue` creates a `ADRealVectorValue` material property.

## Example Input File Syntax

!listing test/tests/materials/generic_materials/generic_constant_real_vector_value.i block=Materials/vector

!syntax parameters /Materials/GenericConstantRealVectorValue

!syntax inputs /Materials/GenericConstantRealVectorValue

!syntax children /Materials/GenericConstantRealVectorValue
