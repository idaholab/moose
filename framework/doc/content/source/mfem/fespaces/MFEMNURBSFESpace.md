# MFEMNURBSFESpace

!if! function=hasCapability('mfem')

## Overview

Builds a scalar finite element space of NURBS basis functions, for isogeometric
analysis (IGA). The order of the basis functions is controlled using the
`fec_order` parameter.

The mesh the space is defined on must be a NURBS mesh, so that the knot vectors
defining the basis functions are available; MFEM reads these from mesh files
with an `MFEM NURBS mesh` header. Since the same basis represents both the
geometry and the solution, the geometry of such a mesh is exact and remains so
under refinement, which for a NURBS mesh inserts knots rather than splitting
elements.

The order requested must be at least the order of the NURBS geometry of the
mesh, as MFEM is only able to raise the degree of the mesh knot vectors and not
to lower it. Requesting a higher order builds a superparametric space by degree
elevating a copy of the knot vectors of the mesh, leaving the geometry itself
unchanged.

Note that MFEM is unable to refine a NURBS mesh once it has been partitioned, so
the `parallel_refine` parameter of [MFEMMesh.md] cannot be used with such a mesh;
use `uniform_refine` (equivalently, `serial_refine`) instead.

## Relationship to MFEMGenericFESpace

An isoparametric NURBS space, whose basis functions are of the same order as the
geometry of the mesh, can equally be built with [MFEMGenericFESpace.md] by
setting `fec_name` to `NURBS` followed by that order, and doing so gives the same
space and the same solution. This class exists for the two things that cannot be
expressed that way:

- Requesting an order higher than the geometry builds a superparametric space,
  which requires degree elevating a copy of the mesh knot vectors and handing the
  result to the finite element space. `MFEMGenericFESpace` always builds the
  space directly on the knot vectors of the mesh, so it can only ever produce the
  isoparametric case.
- The order and the mesh are validated. Requesting an order below that of the
  geometry, or a NURBS space on a mesh with no NURBS geometry at all, is reported
  as an error instead of silently producing a space that is not the one asked
  for.

Use [MFEMGenericFESpace.md] if neither of those matters to you.

## Example Input File Syntax

!listing test/tests/mfem/iga/iga_diffusion.i block=Mesh FESpaces Variables

!syntax parameters /FESpaces/MFEMNURBSFESpace

!syntax inputs /FESpaces/MFEMNURBSFESpace

!syntax children /FESpaces/MFEMNURBSFESpace

!if-end!

!else
!include mfem/mfem_warning.md
