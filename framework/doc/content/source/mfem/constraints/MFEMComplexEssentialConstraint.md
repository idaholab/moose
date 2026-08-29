# MFEMComplexEssentialConstraint

!if! function=hasCapability('mfem')

## Summary

Base class for objects applying essential volumetric constraints to a complex (time-harmonic)
MFEM FE problem.

## Overview

Complex counterpart of [MFEMEssentialConstraint.md]. Classes deriving from
`MFEMComplexEssentialConstraint` act on the trial `mfem::ParComplexGridFunction` of a problem
with `numeric_type = complex`, constraining the real and imaginary components together. The
true degrees of freedom in the constrained subdomain are added to the essential-DOF list, so
the corresponding rows of the discrete system are eliminated in favour of the projected
values.

A complex constraint cannot be used in a real problem, and a real constraint cannot be used in
a complex one; both cases are reported as errors when the constraint is added.

Concrete implementations:

- [MFEMComplexScalarEssentialConstraint.md]
- [MFEMComplexVectorEssentialConstraint.md]
- [MFEMComplexTreeCotreeGaugeEssentialConstraint.md]

!if-end!

!else
!include mfem/mfem_warning.md
