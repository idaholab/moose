# MFEMVectorQuadratureFunction

!if! function=hasCapability('mfem')

## Overview

`MFEMVectorQuadratureFunction` is the vector-valued counterpart of
[MFEMScalarQuadratureFunction.md]: it declares a named vector MFEM coefficient holding precomputed
values of a source vector coefficient at the quadrature points of an
[MFEM QuadratureSpace](https://docs.mfem.org/html/classmfem_1_1QuadratureSpace.html), and can be
used wherever a vector coefficient name is accepted. The source is supplied through the
`vector_coefficient` parameter, and the vector dimension of the stored values is taken from it.

The lazy re-projection behaviour, the `updates` parameter, and the requirement that the consuming
integration rule match the `order` parameter (with MOOSE erroring and suggesting a matching order
otherwise) are all identical to the scalar case; see [MFEMScalarQuadratureFunction.md] for details.

## Example Input File Syntax

!listing test/tests/mfem/kernels/gravity_qf.i

!syntax parameters /QuadratureFunctions/MFEMVectorQuadratureFunction

!syntax inputs /QuadratureFunctions/MFEMVectorQuadratureFunction

!syntax children /QuadratureFunctions/MFEMVectorQuadratureFunction

!if-end!

!else
!include mfem/mfem_warning.md
