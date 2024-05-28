# LinearFVAdvectionDiffusionOutflowBC

## Description

`LinearFVAdvectionDiffusionOutflowBC` will contribute to the system matrix and right hand side
of a linear finite volume system. The contributions can be derived using the
integral of the advective flux over a boundary face ($S_b$) of a boundary element:

!equation
\int\limits_{S_b} \vec{v} \cdot \vec{n} u dS  \approx \vec{v}_f \cdot \vec{n} u_f |S_b|~,

where $\vec{v}_f$, $\vec{n}$ and $|S_b|$ are the outlet face velocity, outward pointing surface vector
and the surface area,

The value of $u_f$ can be computed two different ways depending on the setting of the
[!param](/LinearFVBCs/LinearFVAdvectionDiffusionOutflowBC/use_two_term_expansion) parameter. When the two-term
expansion is enabled the face value is approximated as:

!equation
u_f = u_C + \nabla u_C \cdot \vec{d}_{Cf}~,

where $u_C$ and $\nabla u_C$ are the solution value and gradient in the boundary cell, while
$\vec{d}_{Cf}$ is the vector pointing to the face center from the boundary cell centroid.
When [!param](/LinearFVBCs/LinearFVAdvectionDiffusionOutflowBC/use_two_term_expansion) is disabled the following first-order
approximation is used:

!equation
u_f = u_C.

This boundary condition assumes zero normal gradient contribution to the diffusion terms.

!alert note
This boundary condition should only be used for problems which involve advection and/or diffusion
problems.

## Example Syntax

!listing test/tests/linearfvkernels/advection/advection-2d.i block=LinearFVBCs

!syntax parameters /LinearFVBCs/LinearFVAdvectionDiffusionOutflowBC

!syntax inputs /LinearFVBCs/LinearFVAdvectionDiffusionOutflowBC

!syntax children /LinearFVBCs/LinearFVAdvectionDiffusionOutflowBC
