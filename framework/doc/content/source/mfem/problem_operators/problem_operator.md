# ProblemOperator

## Summary

`ProblemOperator` objects are `mfem::Operators` that are called inside `MFEMExecutioner` to solve a
step of the FE problem and update the `mfem::BlockVector` storing the true degrees of freedom of all
variables.

For more information on usage, see `MFEMExecutioner` and its derived classes.
