# FEValuePenaltyBC
!syntax description /BCs/FEValuePenaltyBC

This `BC` provides *strongly encouraged* FE-based Dirichlet boundary condition. It subclasses `FunctionPenaltyDirichletBC` to perform the actual work. The only differences in `FEValuePenaltyBC` are:
1) it ensures that the provided `Function` is a subclass of `FunctionSeries`
2) it configures the provided `FunctionSeries` to use memoization (caching) for FE evaluations to improve performance


!syntax parameters /BCs/FEValuePenaltyBC

!syntax inputs /BCs/FEValuePenaltyBC

!syntax children /BCs/FEValuePenaltyBC
