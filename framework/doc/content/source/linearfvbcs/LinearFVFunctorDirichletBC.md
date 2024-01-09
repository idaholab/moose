# LinearFVFunctorDirichletBC

## Description

`LinearFVFunctorDirichletBC` will specify the value of a field at the boundary.
The value will be determined by a `Functor` (through the [!param](/FVBCs/FVFunctorDirichletBC/functor) parameter).

## Example Syntax

In this example the functor, a constant value of 10, is defined using a [GenericFunctorMaterial.md].

!listing test/tests/fixmeee/fv_functor_dirichlet/fv_functor_dirichlet.i block=FVBCs

!syntax parameters /LinearFVBCs/LinearFVFunctorDirichletBC

!syntax inputs /LinearFVBCs/LinearFVFunctorDirichletBC

!syntax children /LinearFVBCs/LinearFVFunctorDirichletBC
