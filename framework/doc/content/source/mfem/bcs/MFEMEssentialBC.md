# MFEMEssentialBC

!if! function=hasCapability('mfem')

## Summary

Base class for objects applying essential boundary conditions to an MFEM FE problem.

## Overview

Classes deriving from `MFEMEssentialBC` are used for the application of Dirichlet-like BCs that
remove degrees of freedom from the problem on the specified boundary. These are commonly used when
strongly constraining the values a solution may take on boundaries.

Two essential BCs on the same variable must not constrain the same true degree of freedom,
even if their prescribed values agree. Different boundaries can share degrees of freedom at
their edges or corners. To prescribe the same value on such boundaries, use a single BC
with multiple `boundary` entries.

!if-end!

!else
!include mfem/mfem_warning.md
