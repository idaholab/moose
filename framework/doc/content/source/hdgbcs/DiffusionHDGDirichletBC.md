# DiffusionHDGDirichletBC

This class has some conceptual similarities to [FunctionDirichletBC.md], however, the
Dirichlet condition described by
[!param](/HDGBCs/DiffusionHDGDirichletBC/functor) is
imposed weakly for a hybridized discontinuous Galerkin discretization instead of
strongly for a continuous Galerkin discretization. This boundary condition
should be paired with a [DiffusionHDGKernel.md].

!syntax parameters /HDGBCs/DiffusionHDGDirichletBC

!syntax inputs /HDGBCs/DiffusionHDGDirichletBC

!syntax children /HDGBCs/DiffusionHDGDirichletBC
