# LinearFVAdvectionDiffusionExtrapolatedBC

## Description

`LinearFVAdvectionDiffusionExtrapolatedBC` will contribute to the system matrix and right hand side
of a linear finite volume system.

To approximate the boundary value ($u_f$) two different approaches are supported depending on the setting of the
[!param](/LinearFVBCs/LinearFVAdvectionDiffusionExtrapolatedBC/use_two_term_expansion) parameter. When the two-term
expansion is enabled the face value is approximated as:

!equation
u_f = u_C + \nabla u_C \cdot \vec{d}_{Cf}~,

where $u_C$ and $\nabla u_C$ are the solution value and gradient in the boundary cell, while
$\vec{d}_{Cf}$ is the vector pointing to the face center from the boundary cell centroid.
When [!param](/LinearFVBCs/LinearFVAdvectionDiffusionExtrapolatedBC/use_two_term_expansion) is disabled the following first-order
approximation is used:

!equation
u_f = u_C.

The contributions to the system matrix and right hand side resulting from the boundary value and the
boundary normal gradient are computed accordingly.

!alert note
This boundary condition should only be used for problems which involve advection and/or diffusion
problems.

## Example Syntax

!listing test/tests/linearfvkernels/advection/advection-2d.i block=LinearFVBCs

!syntax parameters /LinearFVBCs/LinearFVAdvectionDiffusionExtrapolatedBC

!syntax inputs /LinearFVBCs/LinearFVAdvectionDiffusionExtrapolatedBC

!syntax children /LinearFVBCs/LinearFVAdvectionDiffusionExtrapolatedBC
