# LinearFVFunctorDirichletBC

## Description

`LinearFVFunctorDirichletBC` will specify the value of a field at the boundary.
The value will be determined by a `Functor`
(through the [!param](/LinearFVBCs/LinearFVFunctorDirichletBC/functor) parameter).

## Example Syntax

In this example the functor is defined using a [MooseParsedFunction.md].

!listing test/tests/linearfvkernels/diffusion/diffusion-2d.i block=LinearFVBCs

!syntax parameters /LinearFVBCs/LinearFVFunctorDirichletBC

!syntax inputs /LinearFVBCs/LinearFVFunctorDirichletBC

!syntax children /LinearFVBCs/LinearFVFunctorDirichletBC
