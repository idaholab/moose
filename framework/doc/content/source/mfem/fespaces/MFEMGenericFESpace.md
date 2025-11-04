# MFEMGenericFESpace

!if! function=hasCapability('mfem')

## Summary

!syntax description /FESpaces/MFEMGenericFESpace

## Overview

This is a low-level routine which allows the user maximum control over
the finite element space being constructed. It takes the name of the
finite element collection and what MFEM calls "vdim" (the number of
degrees of freedom per basis function); these are the same
arguments as one would use with raw MFEM. Information on how to
construct the name of a finite element collection can be [found in the
MFEM documentation](https://docs.mfem.org/html/classmfem_1_1FiniteElementCollection.html#a15fcfa553d4949eb08f9926ac74d1e80).

Most users will find it easier to use the
[MFEMScalarFESpace](MFEMScalarFESpace.md) and
[MFEMVectorFESpace](MFEMVectorFESpace.md) classes instead. These will
select the appropriate finite element collection and vdim based on
your mesh and a few other parameters. However, not all MFEM finite
element collections are supported by those classes.



## Example Input File Syntax

!syntax parameters /FESpaces/MFEMGenericFESpace

!syntax inputs /FESpaces/MFEMGenericFESpace

!syntax children /FESpaces/MFEMGenericFESpace

!if-end!

!else
!include mfem/mfem_warning.md
