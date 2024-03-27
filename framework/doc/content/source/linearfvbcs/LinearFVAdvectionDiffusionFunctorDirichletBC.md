# LinearFVAdvectionDiffusionFunctorDirichletBC

## Description

`LinearFVAdvectionDiffusionFunctorDirichletBC` will specify the value of a field at the boundary.
The value will be determined by a `Functor`
(through the [!param](/LinearFVBCs/LinearFVAdvectionDiffusionFunctorDirichletBC/functor) parameter).

!alert note
This boundary condition should only be used for problems which involve advection and/or diffusion
problems.

## Example Syntax

In this example the functor is defined using a [MooseParsedFunction.md].

!listing test/tests/linearfvkernels/diffusion/diffusion-2d.i block=LinearFVBCs

!syntax parameters /LinearFVBCs/LinearFVAdvectionDiffusionFunctorDirichletBC

!syntax inputs /LinearFVBCs/LinearFVAdvectionDiffusionFunctorDirichletBC

!syntax children /LinearFVBCs/LinearFVAdvectionDiffusionFunctorDirichletBC
