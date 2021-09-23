# DGFunctionDiffusionDirichletBC

!syntax description /BCs/DGFunctionDiffusionDirichletBC

Note that these boundary conditions are specific to DG and to a diffusion problem. Using a [syntax/Functions/index.md]
for the Dirichlet boundary conditions means that the spatial and time dependence is either known or imposed.

More information about Dirichlet boundary conditions and their mathematical meaning may be found in the
[DirichletBC.md] documentation, and more information may be found about the discontinuous Galerkin
discretization in the [DGKernels documentation](syntax/DGKernels/index.md)

## Example input syntax

In this example, a 2D diffusion problem is solved with DG. A Dirichlet boundary condition is imposed
on all boundaries using the `exact_fn` function. The `epsilon` and `sigma` parameters are DG parameters.

!listing test/tests/dgkernels/2d_diffusion_dg/2d_diffusion_dg_test.i block=BCs

!syntax parameters /BCs/DGFunctionDiffusionDirichletBC

!syntax inputs /BCs/DGFunctionDiffusionDirichletBC

!syntax children /BCs/DGFunctionDiffusionDirichletBC
