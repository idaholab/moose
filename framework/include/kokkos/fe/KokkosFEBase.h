//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTypes.h"

namespace Moose::Kokkos
{

// ── Element topology tags ─────────────────────────────────────────────────────
// Zero-size, zero-cost structs used as compile-time template parameters.
// They carry no data; their sole purpose is to select the right FEEvaluator
// specialization at compile time.

struct Edge2Tag  {}; // 1D, 2-node linear edge
struct Edge3Tag  {}; // 1D, 3-node quadratic edge
struct Tri3Tag   {}; // 2D, 3-node linear triangle
struct Tri6Tag   {}; // 2D, 6-node quadratic triangle
struct Quad4Tag  {}; // 2D, 4-node bilinear quadrilateral
struct Quad8Tag  {}; // 2D, 8-node serendipity quadrilateral
struct Quad9Tag  {}; // 2D, 9-node biquadratic quadrilateral
struct Tet4Tag   {}; // 3D, 4-node linear tetrahedron
struct Tet10Tag  {}; // 3D, 10-node quadratic tetrahedron
struct Hex8Tag   {}; // 3D, 8-node trilinear hexahedron
struct Hex20Tag  {}; // 3D, 20-node serendipity hexahedron
struct Hex27Tag  {}; // 3D, 27-node triquadratic hexahedron

// ── FE family tags ────────────────────────────────────────────────────────────

struct LagrangeTag    {};
struct LagrangeVecTag {};
struct HermiteTag     {};

// ── Primary FEEvaluator template ─────────────────────────────────────────────
//
// All uses must be explicit specializations. Every specialization must provide:
//
//   static constexpr unsigned int n_dofs()
//
//   KOKKOS_INLINE_FUNCTION
//   static Real shape(unsigned int i, Real xi, Real eta, Real zeta)
//
//   KOKKOS_INLINE_FUNCTION
//   static Real3 grad_shape(unsigned int i, Real xi, Real eta, Real zeta)
//
// Reference-element coordinate conventions (matching libMesh):
//   Edge:  xi  in [-1, 1]
//   Quad:  (xi, eta)          in [-1,1]^2
//   Hex:   (xi, eta, zeta)    in [-1,1]^3
//   Tri:   (xi, eta)          in unit triangle, xi >= 0, eta >= 0, xi+eta <= 1
//   Tet:   (xi, eta, zeta)    in unit tetrahedron
//
// Unused coordinate arguments (e.g. zeta on a 2D element) are accepted but
// ignored, so call sites can always pass all three without special-casing.

template <typename FamilyTag, typename ElemTag>
struct FEEvaluator; // forward declaration only; instantiation requires a specialization

} // namespace Moose::Kokkos
