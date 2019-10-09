# UpperBoundNodalKernel

!syntax description /NodalKernels/UpperBoundNodalKernel

## Description

`UpperBoundNodalKernel` is meant to be used to impose a upper bound on a coupled
variable `v`. The variable specified by the `variable` parameter is a Lagrange
multiplier. It should have an order equivalent to that of the coupled variable
`v`. The upper bound is specified using the `upper_bound` parameter. By default
it is 0. An optional parameter is `exclude_boundaries` which can be used to
specify boundary nodes where the `UpperBoundNodalKernel` should not be
applied. This can be useful for avoiding singularities in the preconditioning
matrix that can arise when a constraint is active in the same place that another
constraint (like a `DirichletBC` is also active).

## Example Syntax

!listing test/tests/nodalkernels/constraint_enforcement/upper-bound.i block=NodalKernels

!syntax parameters /NodalKernels/UpperBoundNodalKernel

!syntax inputs /NodalKernels/UpperBoundNodalKernel

!syntax children /NodalKernels/UpperBoundNodalKernel
