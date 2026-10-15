# FunctionLibmeshDirichletBC

## Description

`FunctionLibmeshDirichletBC` imposes the essential boundary condition $u = g(t, \vec{x})$, where $g$
is the MOOSE [Function](syntax/Functions/index.md) named by
[!param](/BCs/FunctionLibmeshDirichletBC/function), sourcing the prescribed value of every degree of
freedom on the boundary from libMesh's Dirichlet constraint machinery. See
[LibmeshDirichletBC](LibmeshDirichletBC.md) for what that projection does and why it differs from
assigning a nodal value, which is the distinction between this object and
[FunctionDirichletBC](FunctionDirichletBC.md).

The prescribed values are reprojected once per solve, at the current time, so a function that depends
on time is imposed at the time the solve is taken at.

The [!param](/BCs/FunctionLibmeshDirichletBC/boundary) parameter has to name sidesets, since the
projection integrates over element sides.

## Example Input Syntax

In this example, Laplace's equation is solved for a second-order HIERARCHIC variable `u` over three
time steps, with [!param](/BCs/FunctionLibmeshDirichletBC/function) set to $g = t + x$ on every side
of the unit square. That function is harmonic, so it is also the exact solution, and the projected
boundary values reproduce it at each time step.

!listing test/tests/bcs/libmesh_dirichlet_bc/function_libmesh_dirichlet_bc.i block=BCs

!syntax parameters /BCs/FunctionLibmeshDirichletBC

!syntax inputs /BCs/FunctionLibmeshDirichletBC

!syntax children /BCs/FunctionLibmeshDirichletBC
