# CoupledForceNodalKernel

!syntax description /NodalKernels/CoupledForceNodalKernel

## Description

The `CoupledForceNodalKernel` applies either a sink or source term
proportional to the value of a coupled variable `v`. An optional parameter
`coef` determines whether the term is a source or a sink and the proprtionality
between the coupled variable and the applied force. When `coef` is positive
(the default value is 1), then the term is a source.

## Example Syntax

!listing test/tests/nodalkernels/constraint_enforcement/lower-bound.i block=NodalKernels

!syntax parameters /NodalKernels/CoupledForceNodalKernel

!syntax inputs /NodalKernels/CoupledForceNodalKernel

!syntax children /NodalKernels/CoupledForceNodalKernel
