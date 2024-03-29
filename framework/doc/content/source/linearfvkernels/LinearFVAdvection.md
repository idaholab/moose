# LinearFVAdvection

## Description

This kernel contributes to the system matrix and the right hand side
of a system which is solved for a linear finite volume variable [MooseLinearVariableFV.md].
The contributions can be derived using the discretized term of the advection term:

!equation
\int_{V_C} \nabla \cdot (u \vec{v}) dV = \sum_f\int\limits_{S_f} u \vec{v}\cdot \vec{n} dS \approx
\sum_f u_f\vec{v}\cdot \vec{n}_f|S_f|,

where $V_C$ is a cell in the mesh, while $\vec{v}$ is a pre-defined
constant advecting velocity that can be supplied through the
[!param](/LinearFVKernels/LinearFVAdvection/velocity) parameter.
The face value of the variable $u_f$ is computed using the user-selected interpolation
technique that can be supplied through the [!param](/LinearFVKernels/LinearFVAdvection/advected_interp_method) parameter.

In the simplest case, using a linear interpolation method and an internal face
for the integration, we get the following matrix contribution to the degree of freedom corresponding
to the variable on $V_C$:

!equation
\vec{v} \cdot \vec{n}_f |S_f| g_{C,f},

where $g_C$ is the geometric interpolation weight for the interpolation. Similarly, the same term
contributes to the degree of freedom on the other side of the face with:

!equation
\vec{v} \cdot \vec{n}_f |S_f| (1-g_{C,f}).

## Example input syntax

This example describes a pure advection problem with a source term on a 2D mesh.

!listing test/tests/linearfvkernels/advection/advection-2d.i block=LinearFVKernels

!syntax parameters /LinearFVKernels/LinearFVAdvection

!syntax inputs /LinearFVKernels/LinearFVAdvection

!syntax children /LinearFVKernels/LinearFVAdvection
