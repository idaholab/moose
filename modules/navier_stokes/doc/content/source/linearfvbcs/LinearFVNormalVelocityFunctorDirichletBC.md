# LinearFVNormalVelocityFunctorDirichletBC

## Description

`LinearFVNormalVelocityFunctorDirichletBC` specifies the normal velocity. This boundary condition is a good choice if you can assume the velocity is entirely parallel to the normal direction at the boundary. A positive value for the normal velocity denotes outflow; a negative value denotes inflow. We anticipate that this boundary condition will primarily be used on inflows since that is primarily where Dirichlet conditions for velocity are imposed.

## Example Syntax

In this example a channel is rotated such that it would require some mental work from the user to predetermine the inlet velocity vector. It is much easier to instead prescribe the normal velocity directly and allow the boundary condition to partition the normal velocity among the normal components.

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d/rotated-channel.i block=LinearFVBCs

!syntax parameters /LinearFVBCs/LinearFVNormalVelocityFunctorDirichletBC

!syntax inputs /LinearFVBCs/LinearFVNormalVelocityFunctorDirichletBC

!syntax children /LinearFVBCs/LinearFVNormalVelocityFunctorDirichletBC
