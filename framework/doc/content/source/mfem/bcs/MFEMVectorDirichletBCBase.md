# MFEMVectorDirichletBCBase

!if! function=hasCapability('mfem')

## Summary

Base class for objects applying essential boundary conditions on vector variables in an MFEM FE problem.

## Overview

Classes deriving from `MFEMVectorDirichletBCBase` are used for the application of Dirichlet-like BCs that
remove degrees of freedom from vector variables in the problem on the specified boundary. These are commonly used when
strongly constraining the values a solution may take on boundaries. The imposed values may vary in space and/or time.

!if-end!

!else
!include mfem/mfem_warning.md
