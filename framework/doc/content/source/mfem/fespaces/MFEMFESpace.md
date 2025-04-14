# MFEMFESpace

!if! function=hasCapability('mfem')

## Summary

An abstract base class for MFEM finite element spaces.

## Overview

Children of this class are used to define a finite element space for
MFEM which one or more `MFEMVariable`s can be defined with respect to.

This class has pure virtual methods `MFEMFESpace::getFECName()` and
`MFEMFESpace::getVDim()`, which child classes must implement. The
first should return a name to be passed to [MFEM factory method
`FiniteElementCollection::New()`](https://docs.mfem.org/html/classmfem_1_1FiniteElementCollection.html#a15fcfa553d4949eb08f9926ac74d1e80).
The second should specify the number of degrees of freedom per basis
function in the finite element space and will be passed as the
argument `vdim` in the
[FiniteElementSpace constructor](https://docs.mfem.org/html/classmfem_1_1FiniteElementSpace.html#ab27f0e0f58113cbdb287c3128773a11d).

Note that the actual `mfem::FiniteElementCollection` and
`mfem::FiniteElementSpace` objects are lazily constructed. They will
be built on the first call to `MFEMFESpace::getFEC()` and
`MFEMFESpace::getFESpace()`, respectively. If they
were built when the class was constructed then it would not be
possible for the child classes to implement their own logic to choose
the appropriate finite element collection name and vdim.

!else
!include mfem/mfem_warning.md
