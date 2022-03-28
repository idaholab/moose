# ArrayHFEMDirichletBC

## Description

This boundary condition applies Dirichlet boundary conditions with HFEM (hybrid finite element method) to all components of an array variable.
Its standard variable version is [HFEMDirichletBC.md].
Different boundary values for components can be assigned.

## Example Input Syntax

!listing test/tests/kernels/hfem/array_dirichlet.i start=[all] end=[] include-end=true

!syntax parameters /BCs/ArrayHFEMDirichletBC

!syntax inputs /BCs/ArrayHFEMDirichletBC

!syntax children /BCs/ArrayHFEMDirichletBC
