# ArrayDirichletBC

## Description

This boundary condition applies Dirichlet boundary conditions to all components of an array variable.
Its standard variable version is [DirichletBC.md].
Different boundary values for components can be assigned.

## Example Input Syntax

!listing tests/kernels/array_kernels/array_diffusion_test.i block=BCs

!syntax parameters /BCs/ArrayDirichletBC

!syntax inputs /BCs/ArrayDirichletBC

!syntax children /BCs/ArrayDirichletBC
