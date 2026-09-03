# MFEMScaledVectorAux

!if! function=hasCapability('mfem')

## Overview

AuxKernel for projecting the product of a scalar coefficient $k$ and a vector coefficient
$\vec u$ onto a vector auxiliary variable, storing $k \vec u$.

Since either coefficient may be a variable, this allows a vector variable to be rescaled by a
spatially varying factor. If the scalar coefficient is declared as a block restricted material
property, the product is zero outside the blocks the property is defined on, since a piecewise
coefficient evaluates to zero on subdomains it has not been assigned to. This is the
recommended way of restricting the result to part of the mesh.

## Example Input File Syntax

!listing test/tests/mfem/auxkernels/scaledvector.i block=AuxKernels

!syntax parameters /AuxKernels/MFEMScaledVectorAux

!syntax inputs /AuxKernels/MFEMScaledVectorAux

!syntax children /AuxKernels/MFEMScaledVectorAux

!if-end!

!else
!include mfem/mfem_warning.md
