# MFEMCutTransitionSubMesh

!if! function=hasCapability('mfem')

## Overview

An `MFEMCutTransitionSubMesh` specifies and builds an `mfem::ParSubMesh` object from a
user-specified (interior or exterior) planar cut boundary in a specified closed volumetric
subdomain, consisting of all elements that:

- Have at least one vertex that lies on the boundary,
- Lie on one side of the boundary, and
- Are members of the user-specified volumetric subdomain.

These elements are collectively referred
to as a 'transition' subdomain of the parent mesh, due to their role in defining minimal domains of
support for scalar 'transition' variables used in some methods to enforce global topological
constraints on domains with non-trivial topologies.  

In addition, `MFEMCutTransitionSubMesh` modifies attributes on the parent `mfem::ParMesh` to allow
the new transition region and its boundary to be referenced by other kernels and boundary conditions
in the problem. Specifically, new domain attribute IDs are added to the mesh, with each unique
domain attribute ID belonging to elements now comprising the transition region mapped to a (unique)
new domain attribute ID. A new boundary attribute ID is also added, to label the boundary of the
transition domain excluding the cut boundary.

For convenience, the following new named attribute sets are added, with names given by the following
user-specified parameters:

- `transition_subdomain`, naming the set of all domain attribute IDs consisting the transition
   region
- `closed_subdomain`, naming the set of all domain attribute IDs comprising entire closed domain
   (including the transition region)
- `transition_subdomain_boundary`, naming the new boundary attribute ID for the boundary of the
   transition domain excluding the cut surface.

Existing attribute sets on the mesh, with one or more members labelling subdomains that are split by
the new transition subdomain, are also updated to add the new subdomain IDs of the transition region
member(s) that previously belonged to those sets. This is to ensure block-restricted properties,
like material coefficients, apply to the sets of elements expected whether or not an
`MFEMCutTransitionSubMesh` object is present in the problem.

Further information on the usage of such subdomains to enforce global topological constraints can be
found in 
[P. Dular. International Compumag Society Newsletter, 7, no. 2 (2000):4-7.](https://hdl.handle.net/2268/191358)

## Example Input File Syntax

!listing test/tests/mfem/submeshes/cut_closed_coil.i block=SubMeshes

!syntax parameters /SubMeshes/MFEMCutTransitionSubMesh

!syntax inputs /SubMeshes/MFEMCutTransitionSubMesh

!syntax children /SubMeshes/MFEMCutTransitionSubMesh

!if-end!

!else
!include mfem/mfem_warning.md
