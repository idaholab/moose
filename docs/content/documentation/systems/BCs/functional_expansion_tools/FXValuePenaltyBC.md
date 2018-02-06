# FX Value Penalty BC

!syntax description /BCs/FXValuePenaltyBC

## Description

This `BC` provides *strongly encouraged* FE-based Dirichlet boundary condition. It subclasses `FunctionPenaltyDirichletBC` to perform the actual work. The only differences in `FXValuePenaltyBC` are:
1) it ensures that the provided `Function` is a subclass of `FunctionSeries`
2) it configures the provided `FunctionSeries` to use memoization (caching) for FX evaluations to improve performance

!syntax parameters /BCs/FXValuePenaltyBC

!syntax inputs /BCs/FXValuePenaltyBC

!syntax children /BCs/FXValuePenaltyBC
