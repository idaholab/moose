# DiffusionHybridizedDirichletBC

This class has some conceptual similarities to [FunctionDirichletBC.md], however, the
Dirichlet condition described by
[!param](/HybridizedBCs/DiffusionHybridizedDirichletBC/function) is
imposed weakly for a hybridized discontinuous Galerkin discretization instead of
strongly for a continuous Galerkin discretization. This boundary condition
should be paired with a [DiffusionHybridizedKernel.md].

!syntax parameters /HybridizedBCs/DiffusionHybridizedDirichletBC

!syntax inputs /HybridizedBCs/DiffusionHybridizedDirichletBC

!syntax children /HybridizedBCs/DiffusionHybridizedDirichletBC
