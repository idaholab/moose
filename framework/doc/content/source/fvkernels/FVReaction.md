# FVReaction

## Description

`FVReaction` implements a simple first-order reaction term with unity rate
coefficient where the rate of reaction is directly proportional to the governing
variable $u$.

`FVReaction` can be used to help set-up variations of advection-diffusion-reaction
equations.

## Example Syntax

The syntax for `FVReaction` is simple, only taking the `type` and `variable`
parameters. An example block is shown below for a diffusion-reaction equation:

!listing test/tests/fvkernels/fv_coupled_var/coupled.i block=FVKernels

!syntax parameters /FVKernels/FVReaction

!syntax inputs /FVKernels/FVReaction

!syntax children /FVKernels/FVReaction
