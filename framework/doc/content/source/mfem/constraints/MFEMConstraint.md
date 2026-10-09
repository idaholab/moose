# MFEMConstraint

!if! function=hasCapability('mfem')

## Summary

Base class for objects constraining an MFEM variable within one or more mesh subdomains.

## Overview

`MFEMConstraint` holds everything a constraint needs that does not depend on whether the
variable it acts on is real or complex: the `variable` it applies to, the `block` restriction
naming the subdomains it acts within, and the validation of those subdomains against the mesh.

Because a real and a complex constraint act on different grid function types
(`mfem::ParGridFunction` and `mfem::ParComplexGridFunction`), neither can implement the
other's interface. They therefore derive from this class as *siblings*, each declaring its own
`ApplyConstraint`:

- [MFEMEssentialConstraint.md] for real problems
- [MFEMComplexEssentialConstraint.md] for complex (time-harmonic) problems

This mirrors the way [MFEMEssentialBC.md] and [MFEMComplexEssentialBC.md] both derive from
`MFEMBoundaryCondition`. Applying a constraint of the wrong kind to a problem is reported as
an error when the constraint is added, rather than being silently ignored.

!if-end!

!else
!include mfem/mfem_warning.md
