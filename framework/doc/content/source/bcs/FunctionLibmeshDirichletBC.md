# FunctionLibmeshDirichletBC

!syntax description /BCs/FunctionLibmeshDirichletBC

## Description

`FunctionLibmeshDirichletBC` imposes the essential boundary condition $u = g(t, \vec{x})$ for a MOOSE
[Function](Functions/index.md), sourcing the prescribed value of every degree of freedom on the
boundary from libMesh's Dirichlet constraint machinery. See [LibmeshDirichletBC.md] for what that
projection does and why it differs from assigning a nodal value, which is the distinction between
this object and [FunctionDirichletBC.md].

The prescribed values are reprojected once per solve, at the current time, so a function that depends
on time is imposed at the time the solve is taken at.

The `boundary` parameter has to name sidesets, since the projection integrates over element sides.

## Example Input Syntax

!listing test/tests/bcs/libmesh_dirichlet_bc/function_libmesh_dirichlet_bc.i block=BCs

!syntax parameters /BCs/FunctionLibmeshDirichletBC

!syntax inputs /BCs/FunctionLibmeshDirichletBC

!syntax children /BCs/FunctionLibmeshDirichletBC
