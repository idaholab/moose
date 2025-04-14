# MFEMEssentialBC

!if! function=hasCapability('mfem')

## Summary

Base class for objects applying essential boundary conditions to an MFEM FE problem.

## Overview

Classes deriving from `MFEMEssentialBC` are used for the application of Dirichlet-like BCs that
remove degrees of freedom from the problem on the specified boundary. These are commonly used when
strongly constraining the values a solution may take on boundaries.

!else
!include mfem/mfem_warning.md
