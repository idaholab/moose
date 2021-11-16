# GenericFunctionVectorFunctorMaterial

!syntax description /Materials/GenericFunctionVectorFunctorMaterial

The functor version of [GenericFunctionFunctorMaterial.md], this can be
used to quickly create simple constant anisotropic material properties, for
testing, for initial survey of a problem or simply because the material
properties do not vary much over the domain explored by the simulation.

## Example Input File Syntax

This example shows both a `GenericFunctionVectorFunctorMaterial` and a
[GenericConstantVectorFunctorMaterial.md] to define two vector material properties using
functions and constants respectively.

!listing test/tests/materials/functor_properties/functor-mat-props.i block=Materials

!syntax parameters /Materials/GenericFunctionVectorFunctorMaterial

!syntax inputs /Materials/GenericFunctionVectorFunctorMaterial

!syntax children /Materials/GenericFunctionVectorFunctorMaterial
