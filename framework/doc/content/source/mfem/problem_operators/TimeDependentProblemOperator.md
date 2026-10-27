# TimeDependentProblemOperator

!if! function=hasCapability('mfem')

## Summary

`TimeDependentProblemOperator` objects are
[`mfem::TimeDependentOperator`](https://docs.mfem.org/html/classmfem_1_1TimeDependentOperator.html) that are called inside
[MFEMProblemSolve.md] to solve a step of the FE problem and update the
[`mfem::BlockVector`](https://docs.mfem.org/html/classmfem_1_1BlockVector.html) storing the true
degrees of freedom of all variables.

For more information on usage, see [MFEMProblemSolve.md] and its usage in the
[MFEMTransient.md] executioner class.

!if-end!

!else
!include mfem/mfem_warning.md
