# ArrayTimeDerivativeNodalKernel

## Description

This class is the same as [TimeDerivativeNodalKernel.md] except instead of being applied to a standard field variable, it's applied to an [array variable](ArrayMooseVariable.md).

## Example Syntax

!listing test/tests/nodalkernels/array-reaction-decay/test.i block=NodalKernels

!syntax parameters /NodalKernels/ArrayTimeDerivativeNodalKernel

!syntax inputs /NodalKernels/ArrayTimeDerivativeNodalKernel

!syntax children /NodalKernels/ArrayTimeDerivativeNodalKernel
