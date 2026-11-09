# MFEMVectorProjectionAux

!if! function=hasCapability('mfem')

## Overview

AuxKernel for projecting a vector coefficient onto a vector auxiliary variable
in, e.g., $H(\mathrm{curl})$ or $H(\mathrm{div})$.

The vector coefficient $\vec u$ may optionally be scaled by a scalar coefficient $k$, set by
the `coefficient` parameter, in which case $k \vec u$ is stored. Since either coefficient may be a
variable, this allows a vector variable to be rescaled by a spatially varying factor. If the scalar
coefficient is declared as a block restricted material property, the product is zero outside the
blocks the property is defined on, since a piecewise coefficient evaluates to zero on subdomains it
has not been assigned to. This is the recommended way of restricting the result to part of the
mesh.

Note that [MFEMGradAux.md] and [MFEMCurlAux.md] should be preferred over projection of vector
coefficients representing the gradient of scalar variables and curl of vector variables respectively
where available for performance reasons.

## Example Input File Syntax

!listing mfem/auxkernels/projection.i block=/AuxKernels

Scaling of the projected vector coefficient by scalar coefficients, including one taking the value
of a postprocessor, is shown below:

!listing test/tests/mfem/auxkernels/scaledvector.i block=AuxKernels

!syntax parameters /AuxKernels/MFEMVectorProjectionAux

!syntax inputs /AuxKernels/MFEMVectorProjectionAux

!syntax children /AuxKernels/MFEMVectorProjectionAux

!if-end!

!else
!include mfem/mfem_warning.md
