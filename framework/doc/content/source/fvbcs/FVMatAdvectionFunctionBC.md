# FVMatAdvectionFunctionBC

!syntax description /FVBCs/FVMatAdvectionFunctionBC

## Overview

The `FVMatAdvectionFunctionBC` class cane be used when building MMS test cases. The
parameter `exact_solution` is required. `FVMatAdvectionFunctionBC` was created to
address the limitation that if a [/FVFunctionDirichletBC.md] is used
to do an MMS study on a pure advection problem, the resulting matrix is singular
due to the way dirichlet boundary conditions are implemented.

## Example Input File Syntax

!listing fvkernels/mms/mat-advection.i block=FVBCs

!syntax parameters /FVBCs/FVMatAdvectionFunctionBC

!syntax inputs /FVBCs/FVMatAdvectionFunctionBC

!syntax children /FVBCs/FVMatAdvectionFunctionBC
