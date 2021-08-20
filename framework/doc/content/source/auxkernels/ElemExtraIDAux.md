# ElemExtraIDAux

## Description

This auxiliary kernel copies element integers into an auxiliary variable by evaluating the variable on quadrature points of any element with the element integer of the element.
Invalid element IDs will be evaluated as *-1.0* in this auxiliary kernel for better visualization and for consistent handling between 32-bits and 64-bits integer types.

!syntax parameters /AuxKernels/ElemExtraIDAux

!syntax inputs /AuxKernels/ElemExtraIDAux

!syntax children /AuxKernels/ElemExtraIDAux
