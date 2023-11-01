# CauchyStressFromNEML2Receiver

!if function=hasNEML2()
!syntax description /Materials/CauchyStressFromNEML2Receiver

## Description

This object assigns the outputs from a mesh-wise batched material update calculated by [`CauchyStressFromNEML2UO`](CauchyStressFromNEML2UO.md) into the corresponding material properties. The outputs from the batched material update include stress, derivative of stress w.r.t. strain, and internal variables.

!if! function=hasNEML2()

## Example Input Syntax

!syntax parameters /Materials/CauchyStressFromNEML2Receiver

!if-end!
