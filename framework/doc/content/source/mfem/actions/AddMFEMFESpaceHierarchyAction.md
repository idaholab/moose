# AddMFEMFESpaceHierarchyAction

!if! function=hasCapability('mfem')

## Overview

Action called to add an MFEM finite element space hierarchy to the problem, parsing content inside
an [MFEMFESpaceHierarchy.md] block in the user input. Only has an effect if the
`Problem` type is set to [MFEMProblem.md].

## Example Input File Syntax

!listing test/tests/mfem/solvers/pmg_diffusion.i block=Problem FESpaces FESpaceHierarchies

!syntax parameters /FESpaceHierarchies/AddMFEMFESpaceHierarchyAction

!if-end!

!else
!include mfem/mfem_warning.md
