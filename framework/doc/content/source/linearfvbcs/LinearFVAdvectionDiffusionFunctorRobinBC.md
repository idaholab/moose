# LinearFVAdvectionDiffusionFunctorRobinBC

## Description

`LinearFVAdvectionDiffusionFunctorRobinBC` specifies a canonical Robin or mixed boundary condition, i.e.
a linear combination of the normal gradient of a field and the field's value at a given boundary. 
The boundary condition is specified using three functors $\alpha$, $\beta$, and $\gamma$ that represent
the boundary condition based on the following expression

!equation
\alpha \nabla \phi_b \cdot \hat{n}_b + \beta \phi_b = \gamma

where $\nabla \phi_b$ and $\phi_b$ are the field gradient and value at the boundary $b$ respectively, and
$\hat{n} _b$ is the normal at the boundary $b$.

!alert note
This boundary condition should only be used for problems which involve advection and/or diffusion
problems.

!alert note
If the coefficient $\alpha \: \rightarrow 0$, certain terms in this boundary condition tend to
infinity/`NaN`, which will prevent simulations from converging. If you are manually setting 
$\alpha$ to zero, consider using `LinearFVAdvectionDiffusionFunctorDirichletBC` instead.

## Example Syntax

In this example the functor is defined using a [MooseParsedFunction.md].

!listing test/tests/linearfvbcs/diffusion-2d-robin.i block=LinearFVBCs

!syntax parameters /LinearFVBCs/LinearFVAdvectionDiffusionFunctorRobinBC

!syntax inputs /LinearFVBCs/LinearFVAdvectionDiffusionFunctorRobinBC

!syntax children /LinearFVBCs/LinearFVAdvectionDiffusionFunctorRobinBC
