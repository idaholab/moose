# ArrayReactionNodalKernel

## Description

This class is the same as [ReactionNodalKernel.md] except instead of being applied to a standard field variable, it's applied to an [array variable](ArrayMooseVariable.md).

## Example Syntax

!listing test/tests/nodalkernels/array-reaction-decay/test.i block=NodalKernels

!syntax parameters /NodalKernels/ArrayReactionNodalKernel

!syntax inputs /NodalKernels/ArrayReactionNodalKernel

!syntax children /NodalKernels/ArrayReactionNodalKernel
