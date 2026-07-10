# ProblemOperatorBuilder

!if! function=hasCapability('mfem')

## Summary

`ProblemOperatorBuilder` objects are builder classes that build problemOperators 
when called inside the `Executioner` block, but are configured and stored inside
the `MFEMProblem`.

These can be used with standard equation system problem operators, or user defined 
custom ones, for example usage on custom operators refer to the Unit-test
`MFEMCustomProblemOperator.C`.

!if-end!

!else
!include mfem/mfem_warning.md
