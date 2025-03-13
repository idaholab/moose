# ProblemOperator

## Summary

`ProblemOperator` objects are
[`mfem::Operator`](https://docs.mfem.org/html/classmfem_1_1Operator.html) that are called inside
[`MFEMExecutioner`](MFEMExecutioner.md) to solve a step of the FE problem and update the
[`mfem::BlockVector`](https://docs.mfem.org/html/classmfem_1_1BlockVector.html) storing the true
degrees of freedom of all variables.

For more information on usage, see [`MFEMExecutioner`](MFEMExecutioner.md) and its derived classes.
