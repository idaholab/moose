
# MortarFrictionalStateAux

## Description

The `MortarFrictionalStateAux` outputs the frictional state of the contact interface nodes. This nodal auxiliary kernel requires the user to
provide the Lagrange multipliers that hold the values of normal contact pressure and frictional tangential pressures. Using a coefficient 
of friction inputted by the user as a reference, this kernel outputs: "1" if the node is not in contact, "2" if the node is in contact and sticking (i.e. frictional force is less than its frictional capacity), and "3" if the node experiences relative tangential velocity, i.e. it is sliding at its frictional capacity.

This kernel can be useful for analyzing complex contact mechanics output in a finite element mesh viewer.

!syntax parameters /AuxKernels/MortarFrictionalStateAux

!syntax inputs /AuxKernels/MortarFrictionalStateAux

!syntax children /AuxKernels/MortarFrictionalStateAux
