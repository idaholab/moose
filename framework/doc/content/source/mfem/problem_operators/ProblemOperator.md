# ProblemOperator

!if! function=hasCapability('mfem')

## Summary

`ProblemOperator` objects are
[`mfem::Operator`](https://docs.mfem.org/html/classmfem_1_1Operator.html) that are called inside
[MFEMProblemSolve.md] to solve a step of the FE problem and update the
[`mfem::BlockVector`](https://docs.mfem.org/html/classmfem_1_1BlockVector.html) storing the true
degrees of freedom of all variables.

For more information on usage, see [MFEMProblemSolve.md] and its usage in the
[MFEMSteady.md] and [MFEMTransient.md] executioner classes.

!if-end!

!else
!include mfem/mfem_warning.md
