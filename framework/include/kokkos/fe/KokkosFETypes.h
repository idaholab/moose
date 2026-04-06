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
  MONOMIAL     = 3,
  MONOMIAL_VEC = 4,
  UNKNOWN      = 5, // sentinel returned by toKokkosFamily for unrecognised libMesh families
  N_FAMILIES
};

// ── Element base-class enum ───────────────────────────────────────────────────
// Collapses all topology variants that share the same geometric shape.
// QUAD4 / QUAD8 / QUAD9 all map to QUAD; TRI3 / TRI6 / TRI7 all map to TRI;
// etc.  Used together with FEFamily and order to identify a physics FE space.

enum class FEElemClass : unsigned int
{
  EDGE    = 0,
  TRI     = 1,
  QUAD    = 2,
  TET     = 3,
  HEX     = 4,
  PRISM   = 5,
  PYRAMID = 6,
  N_CLASSES
};

// ── Shape function space key ──────────────────────────────────────────────────
// Uniquely identifies a physics FE space independent of the geometric topology.
// Trivially copyable; fits in a register (three small integers, no heap).

struct FEShapeKey
{
  FEFamily     family;
  FEElemClass  cls;
  unsigned int order;
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

/// Map an element topology to its base geometric class (order-independent).
/// e.g. QUAD4 / QUAD8 / QUAD9 all return FEElemClass::QUAD.
FEElemClass classFromTopology(FEElemTopology topo);

/// Return the number of DOFs for a physics FE space described by \p key.
/// Supports LAGRANGE (topology-based count) and MONOMIAL (dimension-based
/// total-degree polynomial count).
unsigned int nDofs(FEShapeKey key);

} // namespace Moose::Kokkos
