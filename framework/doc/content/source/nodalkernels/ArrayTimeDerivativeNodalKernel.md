# ArrayTimeDerivativeNodalKernel

## Description

This class is the same as [TimeDerivativeNodalKernel.md] except instead of being applied to a classic MOOSE variable, it's applied to an array variable.

## Example Syntax

!listing test/tests/nodalkernels/array-reaction-decay/test.i block=NodalKernels

!syntax parameters /NodalKernels/ArrayTimeDerivativeNodalKernel

!syntax inputs /NodalKernels/ArrayTimeDerivativeNodalKernel

!syntax children /NodalKernels/ArrayTimeDerivativeNodalKernel
