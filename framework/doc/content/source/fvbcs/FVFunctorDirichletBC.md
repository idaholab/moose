# FVFunctorDirichletBC

## Description

`FVFunctorDirichletBC` will specify the value of a field at the boundary.
The value will be determined by a `Functor`.

!alert note
This boundary condition will only accept regular functors. ADFunctors (such as variables) must be
converted to regular functors using a [FunctorADConverter.md].

## Example Syntax

In this example the functor, a constant value of 10, is defined using a [GenericFunctorMaterial.md].

!listing test/tests/fvbcs/fv_functor_dirichlet/fv_functor_dirichlet.i block=Materials

!listing test/tests/fvbcs/fv_functor_dirichlet/fv_functor_dirichlet.i block=FVBCs

!syntax parameters /FVBCs/FVFunctorDirichletBC

!syntax inputs /FVBCs/FVFunctorDirichletBC

!syntax children /FVBCs/FVFunctorDirichletBC
