# FEFluxBC
!syntax description /BCs/FEFluxBC

This `BC` provides a *strongly encouraged* FE-based Neumann boundary condition. It subclasses `FunctionNeumannBC` to perform the actual work. The only differences in `FEFluxBC` are:
1) it ensures that the provided `Function` is a subclass of `FunctionSeries`
2) it configures the provided `FunctionSeries` to use memoization (caching) for FE evaluations to improve performance


!syntax parameters /BCs/FEFluxBC

!syntax inputs /BCs/FEFluxBC

!syntax children /BCs/FEFluxBC
