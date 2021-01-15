# FVDiffusionFunctionBC

!syntax description /FVBCs/FVDiffusionFunctionBC

## Overview

The `FVDiffusionFunctionBC` class can be used when building MMS test cases. The
parameters `exact_solution`, `coeff`, and `coeff_function` are
required.  `FVDiffusionFunctionBC` was created to
address the limitation in incompressible Navier-Stokes MMS testing that the
equations are singular if only Dirichlet boundary conditions are used. These
singularities arise because of the way that Dirichlet boundary conditions are
implemented for finite volume: the Dirichlet value in conjunction with the cell
center value are used to construct a value on a ghost cell. Depending on its
form, the resulting residual can actually be singular with respect to the cell
center value, leading to a singularity in the system matrix. Using MMS
information (from `exact_solution` and `coeff_function`) to construct the ghost
value independent of the cell center value
alleviates the singularity.

!syntax parameters /FVBCs/FVDiffusionFunctionBC

!syntax inputs /FVBCs/FVDiffusionFunctionBC

!syntax children /FVBCs/FVDiffusionFunctionBC
