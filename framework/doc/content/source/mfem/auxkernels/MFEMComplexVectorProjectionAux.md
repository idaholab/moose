# MFEMComplexVectorProjectionAux

!if! function=hasCapability('mfem')

## Overview

AuxKernel for projecting a pair of vector coefficients onto a complex vector auxiliary variable
in, e.g., $H(\mathrm{curl})$ or $H(\mathrm{div})$.

Both parts of the vector coefficient $\vec u$ may optionally be scaled by a real scalar coefficient
$k$, set by the `coefficient` parameter, in which case $k \vec u$ is stored. As for
[MFEMVectorProjectionAux.md], declaring the scalar coefficient as a block restricted material
property restricts the result to the blocks the property is defined on.

## Example Input File Syntax

!listing test/tests/mfem/auxkernels/scaledvector.i block=AuxKernels/complex_scale_by_function

!syntax parameters /AuxKernels/MFEMComplexVectorProjectionAux

!syntax inputs /AuxKernels/MFEMComplexVectorProjectionAux

!syntax children /AuxKernels/MFEMComplexVectorProjectionAux

!if-end!

!else
!include mfem/mfem_warning.md
