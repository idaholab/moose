# MFEMTransitionSubMesh

!if! function=hasCapability('mfem')

## Overview

An `MFEMTransitionSubMesh` specifies and builds an `mfem::ParSubMesh` object from a
user-specified boundary (an interior cut or an exterior boundary of the mesh) in a specified
closed volumetric subdomain, consisting of all elements that:

- Have at least one vertex that lies on the boundary,
- Lie on a requested side of the boundary, and
- Are members of the user-specified volumetric subdomain.

These elements are collectively referred
to as a 'transition' subdomain of the parent mesh, due to their role in defining minimal domains of
support for scalar 'transition' variables used in some methods to enforce global topological
constraints on domains with non-trivial topologies. A layer several elements thick also serves as
the absorbing region of a perfectly matched layer, as used by [MFEMPMLCurlCurlKernel.md].

## Sides and layer thickness

The boundary need not be planar; it need only be orientable, so that each of its vertices has a well
defined surface normal. That normal is averaged from the boundary faces meeting at the vertex, which
distinguishes the two sides of the boundary: an element lies on the positive side if its centre is
displaced from the boundary vertex along the normal, and on the negative side otherwise.

Two parameters set how many element-thick layers are grown on each side:

- `num_layers_positive`, the number of layers on the positive side, defaulting to one.
- `num_layers_negative`, the number of layers on the negative side, defaulting to zero.

A count of zero grows no layer on that side, so the counts alone select which sides are used: the
default of one positive and zero negative layers produces a single layer on the positive side. An
exterior boundary has elements on one side only, so `num_layers_positive` gives the number of layers
grown inwards and `num_layers_negative` must be zero.

Growth is confined to the subdomains named by `block`, which accepts both numeric attributes and
named attribute sets and applies to every layer, not just the first. A layer grown with
`block = coil` therefore cannot pull in elements of a neighbouring subdomain such as the surrounding
air. Leaving `block` empty applies the object to all subdomains.

In addition, `MFEMTransitionSubMesh` modifies attributes on the parent `mfem::ParMesh` to allow
the new transition region and its boundary to be referenced by other kernels and boundary conditions
in the problem. Specifically, new domain attribute IDs are added to the mesh, with each unique
domain attribute ID belonging to elements now comprising the transition region mapped to a (unique)
new domain attribute ID. A new boundary attribute ID is also added, to label the boundary of the
transition domain excluding the boundary surface.

For convenience, the following new named attribute sets are added, with names given by the following
user-specified parameters:

- `transition_subdomain`, naming the set of all domain attribute IDs consisting the transition
   region
- `closed_subdomain`, naming the set of all domain attribute IDs comprising entire closed domain
   (including the transition region)
- `transition_subdomain_boundary`, naming the new boundary attribute ID for the boundary of the
   transition domain excluding the boundary surface.

Existing attribute sets on the mesh, with one or more members labelling subdomains that are split by
the new transition subdomain, are also updated to add the new subdomain IDs of the transition region
member(s) that previously belonged to those sets. This is to ensure block-restricted properties,
like material coefficients, apply to the sets of elements expected whether or not an
`MFEMTransitionSubMesh` object is present in the problem.

Further information on the usage of such subdomains to enforce global topological constraints can be
found in
[P. Dular. International Compumag Society Newsletter, 7, no. 2 (2000):4-7.](https://hdl.handle.net/2268/191358)

## Example Input File Syntax

!listing test/tests/mfem/submeshes/cut_closed_coil.i block=SubMeshes

!syntax parameters /SubMeshes/MFEMTransitionSubMesh

!syntax inputs /SubMeshes/MFEMTransitionSubMesh

!syntax children /SubMeshes/MFEMTransitionSubMesh

!if-end!

!else
!include mfem/mfem_warning.md
