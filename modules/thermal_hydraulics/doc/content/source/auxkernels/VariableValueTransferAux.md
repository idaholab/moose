# VariableValueTransferAux

!syntax description /AuxKernels/VariableValueTransferAux

The variable values can only be set on one boundary at a time and may only
be received from a single boundary as well.

!alert note
This class assumes a first order mesh.

!alert note
The source variable must be a nonlinear variable and must have its degrees of freedom on nodes, as
do Lagrange variables.

!syntax parameters /AuxKernels/VariableValueTransferAux

!syntax inputs /AuxKernels/VariableValueTransferAux

!syntax children /AuxKernels/VariableValueTransferAux
