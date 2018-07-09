# FX Value BC

!syntax description /BCs/FXValueBC

## Description

This `BC` provides *fixed* FE-based Dirichlet boundary condition. It subclasses `FunctionDirichletBC` to perform the actual work. The only differences in `FXValueBC` are:
1) it ensures that the provided `Function` is a subclass of `FunctionSeries`
2) it configures the provided `FunctionSeries` to use memoization (caching) for FX evaluations to improve performance

## Example Input File Syntax

!listing modules/functional_expansion_tools/examples/2D_interface/main.i block=BCs id=input caption=Example use of FXValueBC

!syntax parameters /BCs/FXValueBC

!syntax inputs /BCs/FXValueBC

!syntax children /BCs/FXValueBC
