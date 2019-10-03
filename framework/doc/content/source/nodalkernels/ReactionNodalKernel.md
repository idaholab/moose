# ReactionNodalKernel

!syntax description /NodalKernels/ReactionNodalKernel

## Description

The `ReactionNodalKernel` is meant to apply a simple volumetric sink reacton
term of the form $k*u$ at nodes, where $k$ is an optional user-supplied reaction
rate coeffiecient and $u$ is the variable that the `ReactionNodalKernel` is
being applied to.

## Example Syntax

!listing test/tests/nodalkernels/scaling/scaling.i block=NodalKernels

!syntax parameters /NodalKernels/ReactionNodalKernel

!syntax inputs /NodalKernels/ReactionNodalKernel

!syntax children /NodalKernels/ReactionNodalKernel
