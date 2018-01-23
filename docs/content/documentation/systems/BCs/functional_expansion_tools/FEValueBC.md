# FEValueBC
!syntax description /BCs/FEValueBC

This `BC` provides *fixed* FE-based Dirichlet boundary condition. It subclasses `FunctionDirichletBC` to perform the actual work. The only differences in `FEValueBC` are:
1) it ensures that the provided `Function` is a subclass of `FunctionSeries`
2) it configures the provided `FunctionSeries` to use memoization (caching) for FE evaluations to improve performance


!syntax parameters /BCs/FEValueBC

!syntax inputs /BCs/FEValueBC

!syntax children /BCs/FEValueBC
