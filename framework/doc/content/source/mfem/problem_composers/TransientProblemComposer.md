# TransientProblemComposer

!if! function=hasCapability('mfem')

## Summary

`TransientProblemComposer` is the builder class for all transient equation system
`ProblemOperator`s when called inside the `MFEMTransient` block. The 
`TimeDependentEquationSystemProblemOperator` is built using this class.
 It is configured and stored inside the `MFEMProblem`.

!if-end!

!else
!include mfem/mfem_warning.md
