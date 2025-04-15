# MFEMScalarFESpace

!if! function=hasCapability('mfem')

## Summary

!syntax description /FESpaces/MFEMScalarFESpace

## Overview

This is a convenience class for building finite element spaces to
represent scalar variables. The family of shape functions is selected
from the `fec_type` parameter, and the order is controlled using the
`fec_order` parameter.

If you need a finite element space that can't be constructed using the
options available in this class, you can use
[MFEMGenericFESpace](MFEMGenericFESpace.md) instead.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion_partial.i block=FESpaces Variables

!syntax parameters /FESpaces/MFEMScalarFESpace

!syntax inputs /FESpaces/MFEMScalarFESpace

!syntax children /FESpaces/MFEMScalarFESpace

!if-end!

!else
!include mfem/mfem_warning.md
