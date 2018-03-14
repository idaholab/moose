# FXFluxBC

!syntax description /BCs/FXFluxBC

## Description

This `BC` provides a *strongly encouraged* FE-based Neumann boundary condition. It subclasses
`FunctionNeumannBC` to perform the actual work. The only differences in `FXFluxBC` are:

1. it ensures that the provided `Function` is a subclass of `FunctionSeries`
2. it configures the provided `FunctionSeries` to use memoization (caching) for FX evaluations to
   improve performance

## Example Input File Syntax

!listing modules/functional_expansion_tools/examples/2D_interface/main.i block=BCs id=input caption=Example use of FXFluxBC

!syntax parameters /BCs/FXFluxBC

!syntax inputs /BCs/FXFluxBC

!syntax children /BCs/FXFluxBC
