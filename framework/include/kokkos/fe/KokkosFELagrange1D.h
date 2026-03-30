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

// Reference-element coordinate conventions (libMesh-compatible):
//   EDGE2/EDGE3: xi ∈ [-1, 1]
//
// EDGE3 node ordering (libMesh non-sequential):
//   index 0 → xi = -1   (left node)
//   index 1 → xi = +1   (right node)
//   index 2 → xi =  0   (midpoint)

namespace Moose::Kokkos
{

// ── EDGE2 (linear edge, 2 nodes) ─────────────────────────────────────────────

template <>
struct FEEvaluator<LagrangeTag, Edge2Tag>
{
  static constexpr unsigned int n_dofs() { return 2; }

#ifdef MOOSE_KOKKOS_SCOPE
  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return 0.5 * (1.0 - xi);
      case 1: return 0.5 * (1.0 + xi);
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real /*xi*/, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return Real3(-0.5, 0.0, 0.0);
      case 1: return Real3( 0.5, 0.0, 0.0);
      default: return Real3(0.0, 0.0, 0.0);
    }
  }
#endif
};

// ── EDGE3 (quadratic edge, 3 nodes) ──────────────────────────────────────────
// Node ordering matches libMesh: 0→left(-1), 1→right(+1), 2→mid(0)
//   L_0(xi) = 0.5*xi*(xi-1)   dL_0/dxi = xi - 0.5
//   L_1(xi) = 0.5*xi*(xi+1)   dL_1/dxi = xi + 0.5
//   L_2(xi) = 1 - xi²         dL_2/dxi = -2*xi

template <>
struct FEEvaluator<LagrangeTag, Edge3Tag>
{
  static constexpr unsigned int n_dofs() { return 3; }

#ifdef MOOSE_KOKKOS_SCOPE
  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return 0.5 * xi * (xi - 1.0);
      case 1: return 0.5 * xi * (xi + 1.0);
      case 2: return 1.0 - xi * xi;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real /*eta*/, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return Real3(xi - 0.5,  0.0, 0.0);
      case 1: return Real3(xi + 0.5,  0.0, 0.0);
      case 2: return Real3(-2.0 * xi, 0.0, 0.0);
      default: return Real3(0.0, 0.0, 0.0);
    }
  }
#endif
};

} // namespace Moose::Kokkos
