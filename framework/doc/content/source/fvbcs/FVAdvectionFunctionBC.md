# FVAdvectionFunctionBC

!syntax description /FVBCs/FVAdvectionFunctionBC

## Overview

The `FVAdvectionFunctionBC` class cane be used when building MMS test cases. The
parameter `exact_solution` is required. `FVAdvectionFunctionBC` was created to
address the limitation that if a [/FVFunctionDirichletBC.md] is used
to do an MMS study on a pure advection problem, the resulting matrix is singular
due to the way dirichlet boundary conditions are implemented.

## Example Input File Syntax

!listing fvkernels/mms/advection.i block=FVBCs

!syntax parameters /FVBCs/FVAdvectionFunctionBC

!syntax inputs /FVBCs/FVAdvectionFunctionBC

!syntax children /FVBCs/FVAdvectionFunctionBC
