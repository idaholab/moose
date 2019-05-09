# ArrayVariableComponent

## Description

This auxiliary kernel copies a component of an array variable into a standard auxiliary variable.
It is noted that when the variable family and order are the same, the copy is exact, otherwise projection or prolongation will be involved.
The copied auxiliary variable can be used in all MOOSE objects that operate on standard variables.

!syntax parameters /AuxKernels/ArrayVariableComponent

!syntax inputs /AuxKernels/ArrayVariableComponent

!syntax children /AuxKernels/ArrayVariableComponent
