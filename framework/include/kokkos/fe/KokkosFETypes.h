//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/enum_elem_type.h"
#include "libmesh/enum_fe_family.h"

namespace Moose::Kokkos
{

// ── Runtime element topology enum ────────────────────────────────────────────
// Integer keys used by initShapeNative() to dispatch to the right FEEvaluator
// specialization at runtime (CPU only). Values are contiguous so they can
// serve directly as array indices.

enum class FEElemTopology : unsigned int
{
  EDGE2  = 0,
  EDGE3  = 1,
  TRI3   = 2,
  TRI6   = 3,
  QUAD4  = 4,
  QUAD8  = 5,
  QUAD9  = 6,
  TET4   = 7,
  TET10  = 8,
  HEX8   = 9,
  HEX20  = 10,
  HEX27  = 11,
  N_TYPES
};

// ── Runtime FE family enum ────────────────────────────────────────────────────

enum class FEFamily : unsigned int
{
  LAGRANGE     = 0,
  LAGRANGE_VEC = 1,
  HERMITE      = 2,
  N_FAMILIES
};

// ── CPU-only conversion helpers ───────────────────────────────────────────────
// These are called once per solve setup, inside initShapeNative() on the host.
// They must not be called from device code.

/// Convert a libMesh ElemType to the Kokkos runtime topology enum.
/// Calls mooseError for element types not supported by the native path.
FEElemTopology toKokkosTopology(libMesh::ElemType t);

/// Convert a libMesh FEFamily to the Kokkos runtime family enum.
/// Calls mooseError for families not supported by the native path.
FEFamily toKokkosFamily(libMesh::FEFamily f);

/// Return the topology of any side of parent element type \p parent.
/// For all standard element types the side topology is uniform across sides,
/// so the side index is not needed.
FEElemTopology getSideTopology(FEElemTopology parent);

/// Return the number of DOFs for a (family, topology) pair.
/// Used by initShapeNative() to size Array2D objects before filling them.
unsigned int nDofs(FEFamily family, FEElemTopology topo);

} // namespace Moose::Kokkos
