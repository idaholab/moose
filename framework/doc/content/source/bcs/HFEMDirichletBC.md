# HFEMDirichletBC

!syntax description /BCs/HFEMDirichletBC

## Description

`HFEMDirichletBC` is used for imposing the Dirichlet boundary with HFEM (hybrid finite element method).
A `HFEMDirichletBC` object is for $\left( u^\ast, \lambda_D \right)_{\partial \Omega_D} + \left( \lambda_D^\ast, u - g \right)_{\partial \Omega_D}$ on the subset of the boundary denoted by $\partial \Omega_D$.
Thus it requires the [!param](/BCs/HFEMDirichletBC/variable) parameter for $u$, the [!param](/BCs/HFEMDirichletBC/lowerd_variable) parameter for $\lambda_D$
and the [!param](/BCs/HFEMDirichletBC/value) corresponds to $g$. Refer to [DGKernels/index.md] for the full HFEM weak form for Poisson's equation.

## Example Input Syntax

!listing test/tests/kernels/hfem/dirichlet.i start=[all] end=[] include-end=true

!syntax parameters /BCs/HFEMDirichletBC

!syntax inputs /BCs/HFEMDirichletBC

!syntax children /BCs/HFEMDirichletBC
