# CopyValueAux

!syntax description /AuxKernels/CopyValueAux

The `CopyValueAux` can be used to make a copy of a field variable, for example to lag them in certain numerical schemes.
Many `AuxKernels` can use variables as arguments, without modifying them, and store the
result in a separate variable. The use of a `CopyValueAux` can often be avoided for that reason.

!syntax parameters /AuxKernels/CopyValueAux

!syntax inputs /AuxKernels/CopyValueAux

!syntax children /AuxKernels/CopyValueAux
