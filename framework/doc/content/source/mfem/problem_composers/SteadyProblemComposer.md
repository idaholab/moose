# SteadyProblemComposer

!if! function=hasCapability('mfem')

## Summary

`SteadyProblemComposer` is the builder class for all steady equation system `ProblemOperator`s
when called inside the `MFEMSteady` block. The `EquationSystemProblemOperator`
,`EigenproblemESProblemOperator` and `ComplexEquationSystemProblemOperator` are built 
using this class. It is configured and stored inside the `MFEMProblem`.

!if-end!

!else
!include mfem/mfem_warning.md
