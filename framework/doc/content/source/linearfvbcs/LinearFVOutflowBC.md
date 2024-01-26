# LinearFVOutflowBC

## Description

`LinearFVOutflowBC` will contribute to the system matrix and right hand side
of a linear finite volume matrix using the following advective flux:

!equation
\int\limits_S \vec{v} \cdot \vec{n} u dS  \approx \vec{v}_f \cdot \vec{n} u_f |S_f|~,

where $\vec{v}_f$, $\vec{n}$ and $|S_f|$ are the outlet face velocity, outward pointing surface vector
and the surface area, respectively. The velocity in this case can be defined using
the [!param](/LinearFVBCs/LinearFVOutflowBC/velocity) parameter.

The value of $u_f$ can be computed two different ways depending on the settings of the
[!param](/LinearFVBCs/LinearFVOutflowBC/use_two_term_expansion) parameter. When the two-term
expansion is enable the face value is approximated as:

!equation
u_f = u_C + \nabla u_C \cdot \vec{d}_Cf~,

where $u_C$ and $\nabla u_C$ are the solution value and gradient in the boundary cell, while
$\vec{d}_Cf$ is the vector pointing to the face center from the boundary cell centroid.
When [!param](/LinearFVBCs/LinearFVOutflowBC/use_two_term_expansion) is disabled the following first-order
approximation is used:

!equation
u_f = u_C.

## Example Syntax

!listing test/tests/linearfvkernels/advection/advection-2d.i block=LinearFVBCs

!syntax parameters /LinearFVBCs/LinearFVOutflowBC

!syntax inputs /LinearFVBCs/LinearFVOutflowBC

!syntax children /LinearFVBCs/LinearFVOutflowBC
