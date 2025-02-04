# DiffusionLHDGDirichletBC

This class has some conceptual similarities to [FunctionDirichletBC.md], however, the
Dirichlet condition described by
[!param](/HDGBCs/DiffusionLHDGDirichletBC/functor) is
imposed weakly for a hybridized discontinuous Galerkin discretization instead of
strongly for a continuous Galerkin discretization. This boundary condition
should be paired with a [DiffusionLHDGKernel.md].

!syntax parameters /HDGBCs/DiffusionLHDGDirichletBC

!syntax inputs /HDGBCs/DiffusionLHDGDirichletBC

!syntax children /HDGBCs/DiffusionLHDGDirichletBC
