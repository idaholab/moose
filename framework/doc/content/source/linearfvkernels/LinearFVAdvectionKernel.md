# LinearFVAdvectionKernel

## Description

This kernel contributes to the system matrix and the right hand side
(when boundary conditions are used) of a system which is solved for a
linear finite volume variable [MooseLinearVariableFV.md].
The contributions can be derived using the numerical integral of the approximated advective flux
on the faces of the cells:

!equation
\int\limits_S u(\vec{r}) \vec{v}\cdot \vec{n} d\vec{r} \approx
u_f\vec{v}|S_f|,

where $\vec{v}$ is a pre-defined constant advecting velocity and can be supplied through
the [!param](/LinearFVKernels/LinearFVAdvectionKernel/velocity) parameter.
The face value of the variable $u_f$ is computed using the user-selected interpolation
technique that can be supplied through the [!param](/LinearFVKernels/LinearFVAdvectionKernel/advected_interp_method) parameter.

## Example input syntax

!listing test/tests/linearfvkernels/advection/advection-2d.i block=LinearFVKernels

!syntax parameters /LinearFVKernels/LinearFVAdvectionKernel

!syntax inputs /LinearFVKernels/LinearFVAdvectionKernel

!syntax children /LinearFVKernels/LinearFVAdvectionKernel
