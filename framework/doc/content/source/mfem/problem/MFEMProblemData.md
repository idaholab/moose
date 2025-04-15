# MFEMProblemData

!if! function=hasCapability('mfem')

## Summary

`MFEMProblemData` stores and owns the required data associated with a snapshot of the MFEM finite element problem.

## Overview

Data stored in the `MFEMProblemData` struct is built and added to by builder methods in
[`MFEMProblem`](problem/MFEMProblem.md).

!listing include/mfem/problem/MFEMProblemData.h

!if-end!

!else
!include mfem/mfem_warning.md
