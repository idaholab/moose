# MFEMComplexEssentialBC

!if! function=hasCapability('mfem')

## Summary

Base class for objects applying complex-valued essential boundary conditions to an MFEM FE problem.

## Overview

Classes deriving from `MFEMComplexEssentialBC` are used for the application of complex-valued Dirichlet-like BCs that
remove degrees of freedom from the problem on the specified boundary. These are commonly used when
strongly constraining the values a solution may take on boundaries.

!if-end!

!else
!include mfem/mfem_warning.md
