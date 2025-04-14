# MFEMVectorFESpace

!if! function=hasCapability('mfem')

## Summary

!syntax description /FESpaces/MFEMVectorFESpace

## Overview

This is a convenience class for building finite element spaces to
represent vector variables. The family of shape functions is selected
from the `fec_type` parameter, and the order is controlled using the
`fec_order` parameter. The number of vector components _in reference
space_ is set by the `range_dim` parameter and defaults to the same as
the dimension of the mesh. The number of vector
components in physical space may differ, if a lower-dimensional mesh
is embedded in a higher dimensional space.

The dimension of the resulting finite element collection/space will be
the same as the dimension of the mesh. The value of `vdim` (the number
of degrees of freedom per basis function) will depend on `fec_type`.

- `H1` and `L2` spaces will have `vdim` set to `range_dim`
- `ND` and `RT` spaces will always have `vdim` set to 1

Note that Nédélec and Raviart-Thomas shape functions do not support
vectors with fewer components than the dimension of the
problem. Furthermore, MFEM does not currently support Nédélec and
Raviart-Thomas finite elements in a 1D reference space but with 2D
vectors.

If you need a finite element space that can't be constructed using the
options available in this class, you can use
[MFEMGenericFESpace](MFEMGenericFESpace.md) instead.

## Example Input File Syntax

!listing test/tests/mfem/kernels/gravity.i block=FESpaces Variables

!syntax parameters /FESpaces/MFEMVectorFESpace

!syntax inputs /FESpaces/MFEMVectorFESpace

!syntax children /FESpaces/MFEMVectorFESpace

!else
!include mfem/mfem_warning.md
