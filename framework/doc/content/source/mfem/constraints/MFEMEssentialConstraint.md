# MFEMEssentialConstraint

!if! function=hasCapability('mfem')

## Summary

Base class for objects applying essential volumetric constraints to an MFEM FE problem.

## Overview

Classes deriving from `MFEMEssentialConstraint` strongly constrain the values a solution may
take within one or more mesh subdomains, given by `block`. The true degrees of freedom in the
constrained subdomain are added to the essential-DOF list, so the corresponding rows of the
discrete system are eliminated in favour of projected values. This is the volumetric analogue
of [MFEMEssentialBC.md].

Concrete implementations:

- [MFEMScalarEssentialConstraint.md] / [MFEMComplexScalarEssentialConstraint.md]
- [MFEMVectorEssentialConstraint.md] / [MFEMComplexVectorEssentialConstraint.md]
- [MFEMTreeCotreeGaugeEssentialConstraint.md] / [MFEMComplexTreeCotreeGaugeEssentialConstraint.md]

!if-end!

!else
!include mfem/mfem_warning.md
