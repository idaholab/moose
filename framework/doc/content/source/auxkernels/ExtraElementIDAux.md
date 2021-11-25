# ExtraElementIDAux

## Description

This auxiliary kernel copies element integers into an auxiliary variable by setting the variable value on quadrature points of any element with the extra element integer ID of the element.
Elements which do not have the extra element ID requested by [!param](/AuxKernels/ExtraElementIDAux/extra_id_name) will be evaluated as *-1.0* in this auxiliary kernel for better visualization and for consistent handling between 32-bits and 64-bits integer types.

!syntax parameters /AuxKernels/ExtraElementIDAux

!syntax inputs /AuxKernels/ExtraElementIDAux

!syntax children /AuxKernels/ExtraElementIDAux
