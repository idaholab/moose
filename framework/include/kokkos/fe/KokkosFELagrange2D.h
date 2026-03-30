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
//   Tri:   xi ≥ 0, eta ≥ 0, xi+eta ≤ 1  (unit triangle)
//   Quad:  (xi, eta) ∈ [-1,1]²

namespace Moose::Kokkos
{

// ── TRI3 (linear triangle, 3 nodes) ──────────────────────────────────────────
// Barycentric: zeta0 = 1-xi-eta,  zeta1 = xi,  zeta2 = eta

template <>
struct FEEvaluator<LagrangeTag, Tri3Tag>
{
  static constexpr unsigned int n_dofs() { return 3; }

#ifdef MOOSE_KOKKOS_SCOPE
  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return 1.0 - xi - eta;
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
      case 0: return Real3(-1.0, -1.0, 0.0);
      case 1: return Real3( 1.0,  0.0, 0.0);
      case 2: return Real3( 0.0,  1.0, 0.0);
      default: return Real3(0.0, 0.0, 0.0);
    }
  }
#endif
};

// ── TRI6 (quadratic triangle, 6 nodes) ───────────────────────────────────────
// Barycentric: z0=1-xi-eta, z1=xi, z2=eta
//   phi_0 = z0*(2*z0-1) = (1-xi-eta)*(1-2*xi-2*eta)
//   phi_1 = z1*(2*z1-1) = xi*(2*xi-1)
//   phi_2 = z2*(2*z2-1) = eta*(2*eta-1)
//   phi_3 = 4*z0*z1     = 4*(1-xi-eta)*xi
//   phi_4 = 4*z1*z2     = 4*xi*eta
//   phi_5 = 4*z2*z0     = 4*eta*(1-xi-eta)

template <>
struct FEEvaluator<LagrangeTag, Tri6Tag>
{
  static constexpr unsigned int n_dofs() { return 6; }

#ifdef MOOSE_KOKKOS_SCOPE
  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    const Real z0 = 1.0 - xi - eta;
    switch (i)
    {
      case 0: return z0 * (2.0 * z0 - 1.0);
      case 1: return xi * (2.0 * xi - 1.0);
      case 2: return eta * (2.0 * eta - 1.0);
      case 3: return 4.0 * z0 * xi;
      case 4: return 4.0 * xi * eta;
      case 5: return 4.0 * eta * z0;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    // d(z0)/d(xi) = d(z0)/d(eta) = -1
    switch (i)
    {
      // d/dxi = 4*xi + 4*eta - 3,  d/deta = 4*xi + 4*eta - 3
      case 0: return Real3(4.0*xi + 4.0*eta - 3.0, 4.0*xi + 4.0*eta - 3.0, 0.0);
      // d/dxi = 4*xi - 1,  d/deta = 0
      case 1: return Real3(4.0*xi - 1.0, 0.0, 0.0);
      // d/dxi = 0,  d/deta = 4*eta - 1
      case 2: return Real3(0.0, 4.0*eta - 1.0, 0.0);
      // d/dxi = 4*(1-2*xi-eta),  d/deta = -4*xi
      case 3: return Real3(4.0*(1.0 - 2.0*xi - eta), -4.0*xi, 0.0);
      // d/dxi = 4*eta,  d/deta = 4*xi
      case 4: return Real3(4.0*eta, 4.0*xi, 0.0);
      // d/dxi = -4*eta,  d/deta = 4*(1-xi-2*eta)
      case 5: return Real3(-4.0*eta, 4.0*(1.0 - xi - 2.0*eta), 0.0);
      default: return Real3(0.0, 0.0, 0.0);
    }
  }
#endif
};

// ── QUAD4 (bilinear quadrilateral, 4 nodes) ───────────────────────────────────
// Tensor product of two EDGE2 bases. libMesh node ordering:
//   node 0: (-1,-1)   node 1: (+1,-1)
//   node 2: (+1,+1)   node 3: (-1,+1)
//
//   phi_i = 0.25*(1 + sx*xi)*(1 + sy*eta)
//   where (sx,sy): 0→(-1,-1), 1→(+1,-1), 2→(+1,+1), 3→(-1,+1)

template <>
struct FEEvaluator<LagrangeTag, Quad4Tag>
{
  static constexpr unsigned int n_dofs() { return 4; }

#ifdef MOOSE_KOKKOS_SCOPE
  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return 0.25 * (1.0 - xi) * (1.0 - eta);
      case 1: return 0.25 * (1.0 + xi) * (1.0 - eta);
      case 2: return 0.25 * (1.0 + xi) * (1.0 + eta);
      case 3: return 0.25 * (1.0 - xi) * (1.0 + eta);
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return Real3(-0.25*(1.0-eta), -0.25*(1.0-xi), 0.0);
      case 1: return Real3( 0.25*(1.0-eta), -0.25*(1.0+xi), 0.0);
      case 2: return Real3( 0.25*(1.0+eta),  0.25*(1.0+xi), 0.0);
      case 3: return Real3(-0.25*(1.0+eta),  0.25*(1.0-xi), 0.0);
      default: return Real3(0.0, 0.0, 0.0);
    }
  }
#endif
};

// ── QUAD8 (serendipity quadrilateral, 8 nodes) ────────────────────────────────
// Corner nodes (0-3) use serendipity shape functions.
// Midside nodes (4-7) use bubble-like edge functions.
// Node ordering:
//   0: (-1,-1)   1: (+1,-1)   2: (+1,+1)   3: (-1,+1)
//   4: ( 0,-1)   5: (+1, 0)   6: ( 0,+1)   7: (-1, 0)

template <>
struct FEEvaluator<LagrangeTag, Quad8Tag>
{
  static constexpr unsigned int n_dofs() { return 8; }

#ifdef MOOSE_KOKKOS_SCOPE
  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    switch (i)
    {
      case 0: return 0.25 * (1.0-xi) * (1.0-eta) * (-1.0-xi-eta);
      case 1: return 0.25 * (1.0+xi) * (1.0-eta) * (-1.0+xi-eta);
      case 2: return 0.25 * (1.0+xi) * (1.0+eta) * (-1.0+xi+eta);
      case 3: return 0.25 * (1.0-xi) * (1.0+eta) * (-1.0-xi+eta);
      case 4: return 0.5  * (1.0-xi*xi) * (1.0-eta);
      case 5: return 0.5  * (1.0+xi)    * (1.0-eta*eta);
      case 6: return 0.5  * (1.0-xi*xi) * (1.0+eta);
      case 7: return 0.5  * (1.0-xi)    * (1.0-eta*eta);
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    switch (i)
    {
      // d/dxi = 0.25*(1-eta)*(2*xi+eta),  d/deta = 0.25*(1-xi)*(xi+2*eta)
      case 0: return Real3(0.25*(1.0-eta)*(2.0*xi+eta),
                           0.25*(1.0-xi)*(xi+2.0*eta),
                           0.0);
      // d/dxi = 0.25*(1-eta)*(2*xi-eta),  d/deta = 0.25*(1+xi)*(2*eta-xi)
      case 1: return Real3(0.25*(1.0-eta)*(2.0*xi-eta),
                           0.25*(1.0+xi)*(2.0*eta-xi),
                           0.0);
      // d/dxi = 0.25*(1+eta)*(2*xi+eta),  d/deta = 0.25*(1+xi)*(xi+2*eta)
      case 2: return Real3(0.25*(1.0+eta)*(2.0*xi+eta),
                           0.25*(1.0+xi)*(xi+2.0*eta),
                           0.0);
      // d/dxi = 0.25*(1+eta)*(2*xi-eta),  d/deta = 0.25*(1-xi)*(2*eta-xi)
      case 3: return Real3(0.25*(1.0+eta)*(2.0*xi-eta),
                           0.25*(1.0-xi)*(2.0*eta-xi),
                           0.0);
      // d/dxi = -xi*(1-eta),  d/deta = -0.5*(1-xi²)
      case 4: return Real3(-xi*(1.0-eta), -0.5*(1.0-xi*xi), 0.0);
      // d/dxi = 0.5*(1-eta²),  d/deta = -eta*(1+xi)
      case 5: return Real3(0.5*(1.0-eta*eta), -eta*(1.0+xi), 0.0);
      // d/dxi = -xi*(1+eta),  d/deta = 0.5*(1-xi²)
      case 6: return Real3(-xi*(1.0+eta), 0.5*(1.0-xi*xi), 0.0);
      // d/dxi = -0.5*(1-eta²),  d/deta = -eta*(1-xi)
      case 7: return Real3(-0.5*(1.0-eta*eta), -eta*(1.0-xi), 0.0);
      default: return Real3(0.0, 0.0, 0.0);
    }
  }
#endif
};

// ── QUAD9 (biquadratic quadrilateral, 9 nodes) ────────────────────────────────
// Tensor product of two EDGE3 bases. libMesh node ordering:
//   i0[] = {0,1,1,0, 2,1,2,0, 2}
//   i1[] = {0,0,1,1, 0,2,1,2, 2}
//
// 1D basis (libMesh non-sequential ordering):
//   L_0(t) = 0.5*t*(t-1)   dL_0/dt = t - 0.5
//   L_1(t) = 0.5*t*(t+1)   dL_1/dt = t + 0.5
//   L_2(t) = 1 - t²        dL_2/dt = -2*t

template <>
struct FEEvaluator<LagrangeTag, Quad9Tag>
{
  static constexpr unsigned int n_dofs() { return 9; }

#ifdef MOOSE_KOKKOS_SCOPE
  // Helper: 1D Lagrange basis value
  KOKKOS_INLINE_FUNCTION static Real L(unsigned int k, Real t)
  {
    switch (k)
    {
      case 0: return 0.5 * t * (t - 1.0);
      case 1: return 0.5 * t * (t + 1.0);
      case 2: return 1.0 - t * t;
      default: return 0.0;
    }
  }

  // Helper: 1D Lagrange basis derivative
  KOKKOS_INLINE_FUNCTION static Real dL(unsigned int k, Real t)
  {
    switch (k)
    {
      case 0: return t - 0.5;
      case 1: return t + 0.5;
      case 2: return -2.0 * t;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    // xi-index then eta-index per node
    static const unsigned int i0[] = {0, 1, 1, 0, 2, 1, 2, 0, 2};
    static const unsigned int i1[] = {0, 0, 1, 1, 0, 2, 1, 2, 2};
    return L(i0[i], xi) * L(i1[i], eta);
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real /*zeta*/)
  {
    static const unsigned int i0[] = {0, 1, 1, 0, 2, 1, 2, 0, 2};
    static const unsigned int i1[] = {0, 0, 1, 1, 0, 2, 1, 2, 2};
    const Real dxi  = dL(i0[i], xi)  * L(i1[i], eta);
    const Real deta = L(i0[i], xi)   * dL(i1[i], eta);
    return Real3(dxi, deta, 0.0);
  }
#endif
};

} // namespace Moose::Kokkos
