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
- [MFEMComplexEssentialConstraint.md] for complex problems

This mirrors the way [MFEMEssentialBC.md] and [MFEMComplexEssentialBC.md] both derive from
`MFEMBoundaryCondition`. Applying a constraint of the wrong kind to a problem is reported as
an error when the constraint is added, rather than being silently ignored.

By default a constraint fixes every degree of freedom of the elements in its subdomains,
including those on the subdomain boundary. Setting `constrain_subdomain_boundary = false`
leaves out the degrees of freedom on that boundary, both on interfaces with other subdomains
and on the exterior boundary of the mesh, so that they are solved for or fixed by an
essential boundary condition instead. The tree-cotree gauges do not take this parameter,
since a gauge has to reach the vertices on the boundary of the region it gauges.

The essential constraints on a variable must be restricted to disjoint sets of subdomains;
an empty `block` covers every subdomain. Constraints sharing a subdomain are reported as an
error, since the degrees of freedom they fix together would depend on the order they are
applied in, most notably for [MFEMTreeCotreeGaugeEssentialConstraint.md].
Since, even when the sets of subdomains are disjoint, the subdomains themselves might only
be interior-disjoint, an error is also reported if a constraint fixes a true degree of freedom
already fixed by another constraint or an essential boundary condition on the same variable.

!if-end!

!else
!include mfem/mfem_warning.md
