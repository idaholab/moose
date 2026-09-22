# MFEMEssentialConstraint

!if! function=hasCapability('mfem')

## Summary

Base class for objects applying essential volumetric constraints to an MFEM FE problem.

## Overview

Classes deriving from `MFEMEssentialConstraint` strongly constrain the values a real-valued
solution may take within one or more mesh subdomains, given by `block`. The true degrees of
freedom in the constrained subdomain are added to the essential-DOF list, so the corresponding
rows of the discrete system are eliminated in favour of projected values. This is the
volumetric analogue of [MFEMEssentialBC.md].

It derives from [MFEMConstraint.md], which carries the `variable` and `block` parameters, and
is a sibling of [MFEMComplexEssentialConstraint.md] for complex (time-harmonic) problems.

Concrete implementations:

- [MFEMScalarEssentialConstraint.md]
- [MFEMVectorEssentialConstraint.md]
- [MFEMTreeCotreeGaugeEssentialConstraint.md]

!if-end!

!else
!include mfem/mfem_warning.md
