//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosFEBase.h"

// ─────────────────────────────────────────────────────────────────────────────
// MONOMIAL FEEvaluator specializations
//
// MONOMIAL uses the complete total-degree polynomial space P_p.  Following
// libMesh's FE<Dim, MONOMIAL>, the basis is parameterised by spatial dimension,
// not element class — TRI and QUAD share Dim2Tag; TET/HEX/PRISM/PYRAMID share
// Dim3Tag.  This gives 3 × 6 = 18 specializations (dims 1/2/3, orders 0–5).
//
// Basis ordering: graded-lex (total degree first, then lexicographic by
// decreasing xi exponent):
//
//   Dim2, order 2: {1, xi, eta, xi², xi·eta, eta²}
//   Dim3, order 2: {1, xi, eta, zeta, xi², xi·eta, xi·zeta, eta², eta·zeta, zeta²}
//
// This matches libMesh::FE<Dim, MONOMIAL>::shape ordering.
//
// Reference-element coordinate conventions (same as libMesh):
//   EDGE: xi  ∈ [-1, 1]
//   TRI:  (xi, eta) on unit triangle (xi ≥ 0, eta ≥ 0, xi+eta ≤ 1)
//   QUAD: (xi, eta) ∈ [-1,1]²
//   TET:  (xi, eta, zeta) on unit tetrahedron
//   HEX:  (xi, eta, zeta) ∈ [-1,1]³
//
// The polynomial expressions are identical regardless of element class within a
// dimension group — the reference coordinate symbols (xi, eta, zeta) refer to
// whatever coordinate system the element uses.
// ─────────────────────────────────────────────────────────────────────────────

namespace Moose::Kokkos
{

// ═══════════════════════════════════════════════════════════════════════════
// Dim1 — EDGE elements (1-D)
// n_dofs = order + 1
// Basis: {1, xi, xi², xi³, ...}
// ═══════════════════════════════════════════════════════════════════════════

template <>
struct FEEvaluator<MonomialTag, Dim1Tag, 0>
{
  static constexpr unsigned int n_dofs() { return 1; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int /*i*/, Real /*xi*/, Real /*eta*/, Real /*zeta*/)
  {
    return 1.0;
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int /*i*/, Real /*xi*/, Real /*eta*/, Real /*zeta*/)
  {
    return {0.0, 0.0, 0.0};
  }
};

template <>
struct FEEvaluator<MonomialTag, Dim1Tag, 1>
{
  static constexpr unsigned int n_dofs() { return 2; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return 1.0;
      case 1: return xi;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real /*xi*/, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return {0.0, 0.0, 0.0};
      case 1: return {1.0, 0.0, 0.0};
      default: return {0.0, 0.0, 0.0};
    }
  }
};

template <>
struct FEEvaluator<MonomialTag, Dim1Tag, 2>
{
  static constexpr unsigned int n_dofs() { return 3; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return 1.0;
      case 1: return xi;
      case 2: return xi * xi;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return {0.0,      0.0, 0.0};
      case 1: return {1.0,      0.0, 0.0};
      case 2: return {2.0 * xi, 0.0, 0.0};
      default: return {0.0, 0.0, 0.0};
    }
  }
};

template <>
struct FEEvaluator<MonomialTag, Dim1Tag, 3>
{
  static constexpr unsigned int n_dofs() { return 4; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return 1.0;
      case 1: return xi;
      case 2: return xi * xi;
      case 3: return xi * xi * xi;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return {0.0,           0.0, 0.0};
      case 1: return {1.0,           0.0, 0.0};
      case 2: return {2.0 * xi,      0.0, 0.0};
      case 3: return {3.0 * xi * xi, 0.0, 0.0};
      default: return {0.0, 0.0, 0.0};
    }
  }
};

template <>
struct FEEvaluator<MonomialTag, Dim1Tag, 4>
{
  static constexpr unsigned int n_dofs() { return 5; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return 1.0;
      case 1: return xi;
      case 2: return xi * xi;
      case 3: return xi * xi * xi;
      case 4: return xi * xi * xi * xi;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return {0.0,                0.0, 0.0};
      case 1: return {1.0,                0.0, 0.0};
      case 2: return {2.0 * xi,           0.0, 0.0};
      case 3: return {3.0 * xi * xi,      0.0, 0.0};
      case 4: return {4.0 * xi * xi * xi, 0.0, 0.0};
      default: return {0.0, 0.0, 0.0};
    }
  }
};

template <>
struct FEEvaluator<MonomialTag, Dim1Tag, 5>
{
  static constexpr unsigned int n_dofs() { return 6; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return 1.0;
      case 1: return xi;
      case 2: return xi * xi;
      case 3: return xi * xi * xi;
      case 4: return xi * xi * xi * xi;
      case 5: return xi * xi * xi * xi * xi;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return {0.0,                     0.0, 0.0};
      case 1: return {1.0,                     0.0, 0.0};
      case 2: return {2.0 * xi,                0.0, 0.0};
      case 3: return {3.0 * xi * xi,           0.0, 0.0};
      case 4: return {4.0 * xi * xi * xi,      0.0, 0.0};
      case 5: return {5.0 * xi * xi * xi * xi, 0.0, 0.0};
      default: return {0.0, 0.0, 0.0};
    }
  }
};

// ═══════════════════════════════════════════════════════════════════════════
// Dim2 — TRI and QUAD elements (2-D)
// n_dofs = (order+1)(order+2)/2
// Graded-lex basis: increasing total degree, within each degree decreasing xi
// exponent: {1, xi, eta, xi², xi·eta, eta², xi³, xi²·eta, xi·eta², eta³, ...}
// ═══════════════════════════════════════════════════════════════════════════

template <>
struct FEEvaluator<MonomialTag, Dim2Tag, 0>
{
  static constexpr unsigned int n_dofs() { return 1; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int /*i*/, Real /*xi*/, Real /*eta*/, Real /*zeta*/)
  {
    return 1.0;
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int /*i*/, Real /*xi*/, Real /*eta*/, Real /*zeta*/)
  {
    return {0.0, 0.0, 0.0};
  }
};

template <>
struct FEEvaluator<MonomialTag, Dim2Tag, 1>
{
  static constexpr unsigned int n_dofs() { return 3; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return 1.0;
      case 1: return xi;
      case 2: return eta;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real /*xi*/, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return {0.0, 0.0, 0.0};
      case 1: return {1.0, 0.0, 0.0};
      case 2: return {0.0, 1.0, 0.0};
      default: return {0.0, 0.0, 0.0};
    }
  }
};

template <>
struct FEEvaluator<MonomialTag, Dim2Tag, 2>
{
  static constexpr unsigned int n_dofs() { return 6; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return 1.0;
      case 1: return xi;
      case 2: return eta;
      case 3: return xi * xi;
      case 4: return xi * eta;
      case 5: return eta * eta;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return {0.0,      0.0,      0.0};
      case 1: return {1.0,      0.0,      0.0};
      case 2: return {0.0,      1.0,      0.0};
      case 3: return {2.0 * xi, 0.0,      0.0};
      case 4: return {eta,      xi,       0.0};
      case 5: return {0.0,      2.0 * eta, 0.0};
      default: return {0.0, 0.0, 0.0};
    }
  }
};

template <>
struct FEEvaluator<MonomialTag, Dim2Tag, 3>
{
  static constexpr unsigned int n_dofs() { return 10; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return 1.0;
      case 1: return xi;
      case 2: return eta;
      case 3: return xi * xi;
      case 4: return xi * eta;
      case 5: return eta * eta;
      case 6: return xi * xi * xi;
      case 7: return xi * xi * eta;
      case 8: return xi * eta * eta;
      case 9: return eta * eta * eta;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return {0.0,           0.0,           0.0};
      case 1: return {1.0,           0.0,           0.0};
      case 2: return {0.0,           1.0,           0.0};
      case 3: return {2.0 * xi,      0.0,           0.0};
      case 4: return {eta,           xi,            0.0};
      case 5: return {0.0,           2.0 * eta,     0.0};
      case 6: return {3.0 * xi * xi, 0.0,           0.0};
      case 7: return {2.0 * xi * eta, xi * xi,      0.0};
      case 8: return {eta * eta,     2.0 * xi * eta, 0.0};
      case 9: return {0.0,           3.0 * eta * eta, 0.0};
      default: return {0.0, 0.0, 0.0};
    }
  }
};

template <>
struct FEEvaluator<MonomialTag, Dim2Tag, 4>
{
  static constexpr unsigned int n_dofs() { return 15; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    switch (i)
    {
      case  0: return 1.0;
      case  1: return xi;
      case  2: return eta;
      case  3: return xi * xi;
      case  4: return xi * eta;
      case  5: return eta * eta;
      case  6: return xi * xi * xi;
      case  7: return xi * xi * eta;
      case  8: return xi * eta * eta;
      case  9: return eta * eta * eta;
      case 10: return xi * xi * xi * xi;
      case 11: return xi * xi * xi * eta;
      case 12: return xi * xi * eta * eta;
      case 13: return xi * eta * eta * eta;
      case 14: return eta * eta * eta * eta;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    switch (i)
    {
      case  0: return {0.0,                0.0,                0.0};
      case  1: return {1.0,                0.0,                0.0};
      case  2: return {0.0,                1.0,                0.0};
      case  3: return {2.0 * xi,           0.0,                0.0};
      case  4: return {eta,                xi,                 0.0};
      case  5: return {0.0,                2.0 * eta,          0.0};
      case  6: return {3.0 * xi * xi,      0.0,                0.0};
      case  7: return {2.0 * xi * eta,     xi * xi,            0.0};
      case  8: return {eta * eta,          2.0 * xi * eta,     0.0};
      case  9: return {0.0,                3.0 * eta * eta,    0.0};
      case 10: return {4.0 * xi * xi * xi,       0.0,                0.0};
      case 11: return {3.0 * xi * xi * eta,      xi * xi * xi,       0.0};
      case 12: return {2.0 * xi * eta * eta,     2.0 * xi * xi * eta, 0.0};
      case 13: return {eta * eta * eta,           3.0 * xi * eta * eta, 0.0};
      case 14: return {0.0,                       4.0 * eta * eta * eta, 0.0};
      default: return {0.0, 0.0, 0.0};
    }
  }
};

template <>
struct FEEvaluator<MonomialTag, Dim2Tag, 5>
{
  static constexpr unsigned int n_dofs() { return 21; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    switch (i)
    {
      case  0: return 1.0;
      case  1: return xi;
      case  2: return eta;
      case  3: return xi * xi;
      case  4: return xi * eta;
      case  5: return eta * eta;
      case  6: return xi * xi * xi;
      case  7: return xi * xi * eta;
      case  8: return xi * eta * eta;
      case  9: return eta * eta * eta;
      case 10: return xi * xi * xi * xi;
      case 11: return xi * xi * xi * eta;
      case 12: return xi * xi * eta * eta;
      case 13: return xi * eta * eta * eta;
      case 14: return eta * eta * eta * eta;
      case 15: return xi * xi * xi * xi * xi;
      case 16: return xi * xi * xi * xi * eta;
      case 17: return xi * xi * xi * eta * eta;
      case 18: return xi * xi * eta * eta * eta;
      case 19: return xi * eta * eta * eta * eta;
      case 20: return eta * eta * eta * eta * eta;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    switch (i)
    {
      case  0: return {0.0,                           0.0,                           0.0};
      case  1: return {1.0,                           0.0,                           0.0};
      case  2: return {0.0,                           1.0,                           0.0};
      case  3: return {2.0 * xi,                      0.0,                           0.0};
      case  4: return {eta,                           xi,                            0.0};
      case  5: return {0.0,                           2.0 * eta,                     0.0};
      case  6: return {3.0 * xi * xi,                 0.0,                           0.0};
      case  7: return {2.0 * xi * eta,                xi * xi,                       0.0};
      case  8: return {eta * eta,                     2.0 * xi * eta,                0.0};
      case  9: return {0.0,                           3.0 * eta * eta,               0.0};
      case 10: return {4.0 * xi * xi * xi,            0.0,                           0.0};
      case 11: return {3.0 * xi * xi * eta,           xi * xi * xi,                  0.0};
      case 12: return {2.0 * xi * eta * eta,          2.0 * xi * xi * eta,           0.0};
      case 13: return {eta * eta * eta,               3.0 * xi * eta * eta,          0.0};
      case 14: return {0.0,                           4.0 * eta * eta * eta,         0.0};
      case 15: return {5.0 * xi * xi * xi * xi,       0.0,                           0.0};
      case 16: return {4.0 * xi * xi * xi * eta,      xi * xi * xi * xi,             0.0};
      case 17: return {3.0 * xi * xi * eta * eta,     2.0 * xi * xi * xi * eta,      0.0};
      case 18: return {2.0 * xi * eta * eta * eta,    3.0 * xi * xi * eta * eta,     0.0};
      case 19: return {eta * eta * eta * eta,         4.0 * xi * eta * eta * eta,    0.0};
      case 20: return {0.0,                           5.0 * eta * eta * eta * eta,   0.0};
      default: return {0.0, 0.0, 0.0};
    }
  }
};

// ═══════════════════════════════════════════════════════════════════════════
// Dim3 — TET, HEX, PRISM, PYRAMID elements (3-D)
// n_dofs = (order+1)(order+2)(order+3)/6
// Graded-lex basis: increasing total degree, then decreasing xi exponent,
// then decreasing eta exponent:
//   {1, xi, eta, zeta, xi², xi·eta, xi·zeta, eta², eta·zeta, zeta², ...}
// ═══════════════════════════════════════════════════════════════════════════

template <>
struct FEEvaluator<MonomialTag, Dim3Tag, 0>
{
  static constexpr unsigned int n_dofs() { return 1; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int /*i*/, Real /*xi*/, Real /*eta*/, Real /*zeta*/)
  {
    return 1.0;
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int /*i*/, Real /*xi*/, Real /*eta*/, Real /*zeta*/)
  {
    return {0.0, 0.0, 0.0};
  }
};

template <>
struct FEEvaluator<MonomialTag, Dim3Tag, 1>
{
  static constexpr unsigned int n_dofs() { return 4; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    switch (i)
    {
      case 0: return 1.0;
      case 1: return xi;
      case 2: return eta;
      case 3: return zeta;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real /*xi*/, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return {0.0, 0.0, 0.0};
      case 1: return {1.0, 0.0, 0.0};
      case 2: return {0.0, 1.0, 0.0};
      case 3: return {0.0, 0.0, 1.0};
      default: return {0.0, 0.0, 0.0};
    }
  }
};

template <>
struct FEEvaluator<MonomialTag, Dim3Tag, 2>
{
  static constexpr unsigned int n_dofs() { return 10; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    switch (i)
    {
      case 0: return 1.0;
      case 1: return xi;
      case 2: return eta;
      case 3: return zeta;
      case 4: return xi * xi;
      case 5: return xi * eta;
      case 6: return xi * zeta;
      case 7: return eta * eta;
      case 8: return eta * zeta;
      case 9: return zeta * zeta;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    switch (i)
    {
      case 0: return {0.0,      0.0,      0.0};
      case 1: return {1.0,      0.0,      0.0};
      case 2: return {0.0,      1.0,      0.0};
      case 3: return {0.0,      0.0,      1.0};
      case 4: return {2.0 * xi, 0.0,      0.0};
      case 5: return {eta,      xi,       0.0};
      case 6: return {zeta,     0.0,      xi};
      case 7: return {0.0,      2.0 * eta, 0.0};
      case 8: return {0.0,      zeta,     eta};
      case 9: return {0.0,      0.0,      2.0 * zeta};
      default: return {0.0, 0.0, 0.0};
    }
  }
};

template <>
struct FEEvaluator<MonomialTag, Dim3Tag, 3>
{
  static constexpr unsigned int n_dofs() { return 20; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    switch (i)
    {
      case  0: return 1.0;
      case  1: return xi;
      case  2: return eta;
      case  3: return zeta;
      case  4: return xi * xi;
      case  5: return xi * eta;
      case  6: return xi * zeta;
      case  7: return eta * eta;
      case  8: return eta * zeta;
      case  9: return zeta * zeta;
      case 10: return xi * xi * xi;
      case 11: return xi * xi * eta;
      case 12: return xi * xi * zeta;
      case 13: return xi * eta * eta;
      case 14: return xi * eta * zeta;
      case 15: return xi * zeta * zeta;
      case 16: return eta * eta * eta;
      case 17: return eta * eta * zeta;
      case 18: return eta * zeta * zeta;
      case 19: return zeta * zeta * zeta;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    switch (i)
    {
      case  0: return {0.0,           0.0,           0.0};
      case  1: return {1.0,           0.0,           0.0};
      case  2: return {0.0,           1.0,           0.0};
      case  3: return {0.0,           0.0,           1.0};
      case  4: return {2.0 * xi,      0.0,           0.0};
      case  5: return {eta,           xi,            0.0};
      case  6: return {zeta,          0.0,           xi};
      case  7: return {0.0,           2.0 * eta,     0.0};
      case  8: return {0.0,           zeta,          eta};
      case  9: return {0.0,           0.0,           2.0 * zeta};
      case 10: return {3.0 * xi * xi, 0.0,           0.0};
      case 11: return {2.0 * xi * eta, xi * xi,      0.0};
      case 12: return {2.0 * xi * zeta, 0.0,         xi * xi};
      case 13: return {eta * eta,     2.0 * xi * eta, 0.0};
      case 14: return {eta * zeta,    xi * zeta,     xi * eta};
      case 15: return {zeta * zeta,   0.0,           2.0 * xi * zeta};
      case 16: return {0.0,           3.0 * eta * eta, 0.0};
      case 17: return {0.0,           2.0 * eta * zeta, eta * eta};
      case 18: return {0.0,           zeta * zeta,   2.0 * eta * zeta};
      case 19: return {0.0,           0.0,           3.0 * zeta * zeta};
      default: return {0.0, 0.0, 0.0};
    }
  }
};

template <>
struct FEEvaluator<MonomialTag, Dim3Tag, 4>
{
  static constexpr unsigned int n_dofs() { return 35; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    switch (i)
    {
      // degree 0
      case  0: return 1.0;
      // degree 1
      case  1: return xi;
      case  2: return eta;
      case  3: return zeta;
      // degree 2
      case  4: return xi * xi;
      case  5: return xi * eta;
      case  6: return xi * zeta;
      case  7: return eta * eta;
      case  8: return eta * zeta;
      case  9: return zeta * zeta;
      // degree 3
      case 10: return xi * xi * xi;
      case 11: return xi * xi * eta;
      case 12: return xi * xi * zeta;
      case 13: return xi * eta * eta;
      case 14: return xi * eta * zeta;
      case 15: return xi * zeta * zeta;
      case 16: return eta * eta * eta;
      case 17: return eta * eta * zeta;
      case 18: return eta * zeta * zeta;
      case 19: return zeta * zeta * zeta;
      // degree 4
      case 20: return xi * xi * xi * xi;
      case 21: return xi * xi * xi * eta;
      case 22: return xi * xi * xi * zeta;
      case 23: return xi * xi * eta * eta;
      case 24: return xi * xi * eta * zeta;
      case 25: return xi * xi * zeta * zeta;
      case 26: return xi * eta * eta * eta;
      case 27: return xi * eta * eta * zeta;
      case 28: return xi * eta * zeta * zeta;
      case 29: return xi * zeta * zeta * zeta;
      case 30: return eta * eta * eta * eta;
      case 31: return eta * eta * eta * zeta;
      case 32: return eta * eta * zeta * zeta;
      case 33: return eta * zeta * zeta * zeta;
      case 34: return zeta * zeta * zeta * zeta;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    switch (i)
    {
      case  0: return {0.0,                0.0,                0.0};
      case  1: return {1.0,                0.0,                0.0};
      case  2: return {0.0,                1.0,                0.0};
      case  3: return {0.0,                0.0,                1.0};
      case  4: return {2.0*xi,             0.0,                0.0};
      case  5: return {eta,                xi,                 0.0};
      case  6: return {zeta,               0.0,                xi};
      case  7: return {0.0,                2.0*eta,            0.0};
      case  8: return {0.0,                zeta,               eta};
      case  9: return {0.0,                0.0,                2.0*zeta};
      case 10: return {3.0*xi*xi,          0.0,                0.0};
      case 11: return {2.0*xi*eta,         xi*xi,              0.0};
      case 12: return {2.0*xi*zeta,        0.0,                xi*xi};
      case 13: return {eta*eta,            2.0*xi*eta,         0.0};
      case 14: return {eta*zeta,           xi*zeta,            xi*eta};
      case 15: return {zeta*zeta,          0.0,                2.0*xi*zeta};
      case 16: return {0.0,                3.0*eta*eta,        0.0};
      case 17: return {0.0,                2.0*eta*zeta,       eta*eta};
      case 18: return {0.0,                zeta*zeta,          2.0*eta*zeta};
      case 19: return {0.0,                0.0,                3.0*zeta*zeta};
      case 20: return {4.0*xi*xi*xi,       0.0,                0.0};
      case 21: return {3.0*xi*xi*eta,      xi*xi*xi,           0.0};
      case 22: return {3.0*xi*xi*zeta,     0.0,                xi*xi*xi};
      case 23: return {2.0*xi*eta*eta,     2.0*xi*xi*eta,      0.0};
      case 24: return {2.0*xi*eta*zeta,    xi*xi*zeta,         xi*xi*eta};
      case 25: return {2.0*xi*zeta*zeta,   0.0,                2.0*xi*xi*zeta};
      case 26: return {eta*eta*eta,        3.0*xi*eta*eta,     0.0};
      case 27: return {eta*eta*zeta,       2.0*xi*eta*zeta,    xi*eta*eta};
      case 28: return {eta*zeta*zeta,      xi*zeta*zeta,       2.0*xi*eta*zeta};
      case 29: return {zeta*zeta*zeta,     0.0,                3.0*xi*zeta*zeta};
      case 30: return {0.0,                4.0*eta*eta*eta,    0.0};
      case 31: return {0.0,                3.0*eta*eta*zeta,   eta*eta*eta};
      case 32: return {0.0,                2.0*eta*zeta*zeta,  2.0*eta*eta*zeta};
      case 33: return {0.0,                zeta*zeta*zeta,     3.0*eta*zeta*zeta};
      case 34: return {0.0,                0.0,                4.0*zeta*zeta*zeta};
      default: return {0.0, 0.0, 0.0};
    }
  }
};

template <>
struct FEEvaluator<MonomialTag, Dim3Tag, 5>
{
  static constexpr unsigned int n_dofs() { return 56; }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    switch (i)
    {
      // degree 0
      case  0: return 1.0;
      // degree 1
      case  1: return xi;
      case  2: return eta;
      case  3: return zeta;
      // degree 2
      case  4: return xi*xi;
      case  5: return xi*eta;
      case  6: return xi*zeta;
      case  7: return eta*eta;
      case  8: return eta*zeta;
      case  9: return zeta*zeta;
      // degree 3
      case 10: return xi*xi*xi;
      case 11: return xi*xi*eta;
      case 12: return xi*xi*zeta;
      case 13: return xi*eta*eta;
      case 14: return xi*eta*zeta;
      case 15: return xi*zeta*zeta;
      case 16: return eta*eta*eta;
      case 17: return eta*eta*zeta;
      case 18: return eta*zeta*zeta;
      case 19: return zeta*zeta*zeta;
      // degree 4
      case 20: return xi*xi*xi*xi;
      case 21: return xi*xi*xi*eta;
      case 22: return xi*xi*xi*zeta;
      case 23: return xi*xi*eta*eta;
      case 24: return xi*xi*eta*zeta;
      case 25: return xi*xi*zeta*zeta;
      case 26: return xi*eta*eta*eta;
      case 27: return xi*eta*eta*zeta;
      case 28: return xi*eta*zeta*zeta;
      case 29: return xi*zeta*zeta*zeta;
      case 30: return eta*eta*eta*eta;
      case 31: return eta*eta*eta*zeta;
      case 32: return eta*eta*zeta*zeta;
      case 33: return eta*zeta*zeta*zeta;
      case 34: return zeta*zeta*zeta*zeta;
      // degree 5
      case 35: return xi*xi*xi*xi*xi;
      case 36: return xi*xi*xi*xi*eta;
      case 37: return xi*xi*xi*xi*zeta;
      case 38: return xi*xi*xi*eta*eta;
      case 39: return xi*xi*xi*eta*zeta;
      case 40: return xi*xi*xi*zeta*zeta;
      case 41: return xi*xi*eta*eta*eta;
      case 42: return xi*xi*eta*eta*zeta;
      case 43: return xi*xi*eta*zeta*zeta;
      case 44: return xi*xi*zeta*zeta*zeta;
      case 45: return xi*eta*eta*eta*eta;
      case 46: return xi*eta*eta*eta*zeta;
      case 47: return xi*eta*eta*zeta*zeta;
      case 48: return xi*eta*zeta*zeta*zeta;
      case 49: return xi*zeta*zeta*zeta*zeta;
      case 50: return eta*eta*eta*eta*eta;
      case 51: return eta*eta*eta*eta*zeta;
      case 52: return eta*eta*eta*zeta*zeta;
      case 53: return eta*eta*zeta*zeta*zeta;
      case 54: return eta*zeta*zeta*zeta*zeta;
      case 55: return zeta*zeta*zeta*zeta*zeta;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    switch (i)
    {
      case  0: return {0.0,                    0.0,                    0.0};
      case  1: return {1.0,                    0.0,                    0.0};
      case  2: return {0.0,                    1.0,                    0.0};
      case  3: return {0.0,                    0.0,                    1.0};
      case  4: return {2.0*xi,                 0.0,                    0.0};
      case  5: return {eta,                    xi,                     0.0};
      case  6: return {zeta,                   0.0,                    xi};
      case  7: return {0.0,                    2.0*eta,                0.0};
      case  8: return {0.0,                    zeta,                   eta};
      case  9: return {0.0,                    0.0,                    2.0*zeta};
      case 10: return {3.0*xi*xi,              0.0,                    0.0};
      case 11: return {2.0*xi*eta,             xi*xi,                  0.0};
      case 12: return {2.0*xi*zeta,            0.0,                    xi*xi};
      case 13: return {eta*eta,                2.0*xi*eta,             0.0};
      case 14: return {eta*zeta,               xi*zeta,                xi*eta};
      case 15: return {zeta*zeta,              0.0,                    2.0*xi*zeta};
      case 16: return {0.0,                    3.0*eta*eta,            0.0};
      case 17: return {0.0,                    2.0*eta*zeta,           eta*eta};
      case 18: return {0.0,                    zeta*zeta,              2.0*eta*zeta};
      case 19: return {0.0,                    0.0,                    3.0*zeta*zeta};
      case 20: return {4.0*xi*xi*xi,           0.0,                    0.0};
      case 21: return {3.0*xi*xi*eta,          xi*xi*xi,               0.0};
      case 22: return {3.0*xi*xi*zeta,         0.0,                    xi*xi*xi};
      case 23: return {2.0*xi*eta*eta,         2.0*xi*xi*eta,          0.0};
      case 24: return {2.0*xi*eta*zeta,        xi*xi*zeta,             xi*xi*eta};
      case 25: return {2.0*xi*zeta*zeta,       0.0,                    2.0*xi*xi*zeta};
      case 26: return {eta*eta*eta,            3.0*xi*eta*eta,         0.0};
      case 27: return {eta*eta*zeta,           2.0*xi*eta*zeta,        xi*eta*eta};
      case 28: return {eta*zeta*zeta,          xi*zeta*zeta,           2.0*xi*eta*zeta};
      case 29: return {zeta*zeta*zeta,         0.0,                    3.0*xi*zeta*zeta};
      case 30: return {0.0,                    4.0*eta*eta*eta,        0.0};
      case 31: return {0.0,                    3.0*eta*eta*zeta,       eta*eta*eta};
      case 32: return {0.0,                    2.0*eta*zeta*zeta,      2.0*eta*eta*zeta};
      case 33: return {0.0,                    zeta*zeta*zeta,         3.0*eta*zeta*zeta};
      case 34: return {0.0,                    0.0,                    4.0*zeta*zeta*zeta};
      case 35: return {5.0*xi*xi*xi*xi,        0.0,                    0.0};
      case 36: return {4.0*xi*xi*xi*eta,       xi*xi*xi*xi,            0.0};
      case 37: return {4.0*xi*xi*xi*zeta,      0.0,                    xi*xi*xi*xi};
      case 38: return {3.0*xi*xi*eta*eta,      2.0*xi*xi*xi*eta,       0.0};
      case 39: return {3.0*xi*xi*eta*zeta,     xi*xi*xi*zeta,          xi*xi*xi*eta};
      case 40: return {3.0*xi*xi*zeta*zeta,    0.0,                    2.0*xi*xi*xi*zeta};
      case 41: return {2.0*xi*eta*eta*eta,     3.0*xi*xi*eta*eta,      0.0};
      case 42: return {2.0*xi*eta*eta*zeta,    2.0*xi*xi*eta*zeta,     xi*xi*eta*eta};
      case 43: return {2.0*xi*eta*zeta*zeta,   xi*xi*zeta*zeta,        2.0*xi*xi*eta*zeta};
      case 44: return {2.0*xi*zeta*zeta*zeta,  0.0,                    3.0*xi*xi*zeta*zeta};
      case 45: return {eta*eta*eta*eta,        4.0*xi*eta*eta*eta,     0.0};
      case 46: return {eta*eta*eta*zeta,       3.0*xi*eta*eta*zeta,    xi*eta*eta*eta};
      case 47: return {eta*eta*zeta*zeta,      2.0*xi*eta*zeta*zeta,   2.0*xi*eta*eta*zeta};
      case 48: return {eta*zeta*zeta*zeta,     xi*zeta*zeta*zeta,      3.0*xi*eta*zeta*zeta};
      case 49: return {zeta*zeta*zeta*zeta,    0.0,                    4.0*xi*zeta*zeta*zeta};
      case 50: return {0.0,                    5.0*eta*eta*eta*eta,    0.0};
      case 51: return {0.0,                    4.0*eta*eta*eta*zeta,   eta*eta*eta*eta};
      case 52: return {0.0,                    3.0*eta*eta*zeta*zeta,  2.0*eta*eta*eta*zeta};
      case 53: return {0.0,                    2.0*eta*zeta*zeta*zeta, 3.0*eta*eta*zeta*zeta};
      case 54: return {0.0,                    zeta*zeta*zeta*zeta,    4.0*eta*zeta*zeta*zeta};
      case 55: return {0.0,                    0.0,                    5.0*zeta*zeta*zeta*zeta};
      default: return {0.0, 0.0, 0.0};
    }
  }
};

} // namespace Moose::Kokkos
