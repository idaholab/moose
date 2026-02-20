# AddMFEMPeriodicBCs

!if! function=hasCapability('mfem')

## Overview

Action called to specify a periodic boundary condition on an MFEM mesh, where one isn't there already.
It parses the `MFEMPeriodic` block in the user input.

## Example Input File Syntax

!listing test/tests/mfem/periodic/periodic_cube_vector.i block=MFEMPeriodic

!syntax parameters /AddMFEMPeriodicBCs

!if-end!

!else
!include mfem/mfem_warning.md
