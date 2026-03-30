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
//   Tet: xi ≥ 0, eta ≥ 0, zeta ≥ 0, xi+eta+zeta ≤ 1  (unit tetrahedron)
//   Hex: (xi, eta, zeta) ∈ [-1,1]³

namespace Moose::Kokkos
{

// ── TET4 (linear tetrahedron, 4 nodes) ───────────────────────────────────────
// Barycentric: z0=1-xi-eta-zeta, z1=xi, z2=eta, z3=zeta

template <>
struct FEEvaluator<LagrangeTag, Tet4Tag>
{
  static constexpr unsigned int n_dofs() { return 4; }

#ifdef MOOSE_KOKKOS_SCOPE
  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    switch (i)
    {
      case 0: return 1.0 - xi - eta - zeta;
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
      case 0: return Real3(-1.0, -1.0, -1.0);
      case 1: return Real3( 1.0,  0.0,  0.0);
      case 2: return Real3( 0.0,  1.0,  0.0);
      case 3: return Real3( 0.0,  0.0,  1.0);
      default: return Real3(0.0, 0.0, 0.0);
    }
  }
#endif
};

// ── TET10 (quadratic tetrahedron, 10 nodes) ───────────────────────────────────
// Barycentric: z0=1-xi-eta-zeta, z1=xi, z2=eta, z3=zeta
//
//   phi_0  = z0*(2*z0-1)
//   phi_1  = z1*(2*z1-1) = xi*(2*xi-1)
//   phi_2  = z2*(2*z2-1) = eta*(2*eta-1)
//   phi_3  = z3*(2*z3-1) = zeta*(2*zeta-1)
//   phi_4  = 4*z0*z1     = 4*(1-xi-eta-zeta)*xi
//   phi_5  = 4*z1*z2     = 4*xi*eta
//   phi_6  = 4*z2*z0     = 4*eta*(1-xi-eta-zeta)
//   phi_7  = 4*z0*z3     = 4*(1-xi-eta-zeta)*zeta
//   phi_8  = 4*z1*z3     = 4*xi*zeta
//   phi_9  = 4*z2*z3     = 4*eta*zeta

template <>
struct FEEvaluator<LagrangeTag, Tet10Tag>
{
  static constexpr unsigned int n_dofs() { return 10; }

#ifdef MOOSE_KOKKOS_SCOPE
  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    const Real z0 = 1.0 - xi - eta - zeta;
    switch (i)
    {
      case 0: return z0  * (2.0*z0   - 1.0);
      case 1: return xi  * (2.0*xi   - 1.0);
      case 2: return eta * (2.0*eta  - 1.0);
      case 3: return zeta* (2.0*zeta - 1.0);
      case 4: return 4.0 * z0 * xi;
      case 5: return 4.0 * xi * eta;
      case 6: return 4.0 * eta * z0;
      case 7: return 4.0 * z0 * zeta;
      case 8: return 4.0 * xi * zeta;
      case 9: return 4.0 * eta * zeta;
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    // dz0/dxi = dz0/deta = dz0/dzeta = -1
    const Real z0 = 1.0 - xi - eta - zeta;
    switch (i)
    {
      // d phi_0/d(xi,eta,zeta) = 4*(xi+eta+zeta) - 3  (all three equal)
      case 0:
      {
        const Real v = 4.0*(xi + eta + zeta) - 3.0;
        return Real3(v, v, v);
      }
      case 1: return Real3(4.0*xi - 1.0, 0.0, 0.0);
      case 2: return Real3(0.0, 4.0*eta - 1.0, 0.0);
      case 3: return Real3(0.0, 0.0, 4.0*zeta - 1.0);
      // phi_4 = 4*z0*xi  → d/dxi = 4*(z0-xi) = 4*(1-2xi-eta-zeta)
      case 4: return Real3( 4.0*(1.0-2.0*xi-eta-zeta), -4.0*xi, -4.0*xi);
      case 5: return Real3( 4.0*eta, 4.0*xi, 0.0);
      // phi_6 = 4*eta*z0 → d/deta = 4*(z0-eta) = 4*(1-xi-2eta-zeta)
      case 6: return Real3(-4.0*eta, 4.0*(1.0-xi-2.0*eta-zeta), -4.0*eta);
      // phi_7 = 4*z0*zeta → d/dzeta = 4*(z0-zeta) = 4*(1-xi-eta-2zeta)
      case 7: return Real3(-4.0*zeta, -4.0*zeta, 4.0*(1.0-xi-eta-2.0*zeta));
      case 8: return Real3(4.0*zeta, 0.0, 4.0*xi);
      case 9: return Real3(0.0, 4.0*zeta, 4.0*eta);
      default: return Real3(0.0, 0.0, 0.0);
    }
  }
#endif
};

// ── HEX8 (trilinear hexahedron, 8 nodes) ─────────────────────────────────────
// Tensor product of three EDGE2 bases.
// Node ordering (same as libMesh):
//   0:(-1,-1,-1)  1:(+1,-1,-1)  2:(+1,+1,-1)  3:(-1,+1,-1)
//   4:(-1,-1,+1)  5:(+1,-1,+1)  6:(+1,+1,+1)  7:(-1,+1,+1)
//
//   phi_i = 0.125*(1+sx*xi)*(1+sy*eta)*(1+sz*zeta)

template <>
struct FEEvaluator<LagrangeTag, Hex8Tag>
{
  static constexpr unsigned int n_dofs() { return 8; }

#ifdef MOOSE_KOKKOS_SCOPE
  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    switch (i)
    {
      case 0: return 0.125*(1.0-xi)*(1.0-eta)*(1.0-zeta);
      case 1: return 0.125*(1.0+xi)*(1.0-eta)*(1.0-zeta);
      case 2: return 0.125*(1.0+xi)*(1.0+eta)*(1.0-zeta);
      case 3: return 0.125*(1.0-xi)*(1.0+eta)*(1.0-zeta);
      case 4: return 0.125*(1.0-xi)*(1.0-eta)*(1.0+zeta);
      case 5: return 0.125*(1.0+xi)*(1.0-eta)*(1.0+zeta);
      case 6: return 0.125*(1.0+xi)*(1.0+eta)*(1.0+zeta);
      case 7: return 0.125*(1.0-xi)*(1.0+eta)*(1.0+zeta);
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    switch (i)
    {
      // Node 0: sx=-1, sy=-1, sz=-1
      case 0: return Real3(-0.125*(1.0-eta)*(1.0-zeta),
                           -0.125*(1.0-xi) *(1.0-zeta),
                           -0.125*(1.0-xi) *(1.0-eta));
      // Node 1: sx=+1, sy=-1, sz=-1
      case 1: return Real3( 0.125*(1.0-eta)*(1.0-zeta),
                           -0.125*(1.0+xi) *(1.0-zeta),
                           -0.125*(1.0+xi) *(1.0-eta));
      // Node 2: sx=+1, sy=+1, sz=-1
      case 2: return Real3( 0.125*(1.0+eta)*(1.0-zeta),
                            0.125*(1.0+xi) *(1.0-zeta),
                           -0.125*(1.0+xi) *(1.0+eta));
      // Node 3: sx=-1, sy=+1, sz=-1
      case 3: return Real3(-0.125*(1.0+eta)*(1.0-zeta),
                            0.125*(1.0-xi) *(1.0-zeta),
                           -0.125*(1.0-xi) *(1.0+eta));
      // Node 4: sx=-1, sy=-1, sz=+1
      case 4: return Real3(-0.125*(1.0-eta)*(1.0+zeta),
                           -0.125*(1.0-xi) *(1.0+zeta),
                            0.125*(1.0-xi) *(1.0-eta));
      // Node 5: sx=+1, sy=-1, sz=+1
      case 5: return Real3( 0.125*(1.0-eta)*(1.0+zeta),
                           -0.125*(1.0+xi) *(1.0+zeta),
                            0.125*(1.0+xi) *(1.0-eta));
      // Node 6: sx=+1, sy=+1, sz=+1
      case 6: return Real3( 0.125*(1.0+eta)*(1.0+zeta),
                            0.125*(1.0+xi) *(1.0+zeta),
                            0.125*(1.0+xi) *(1.0+eta));
      // Node 7: sx=-1, sy=+1, sz=+1
      case 7: return Real3(-0.125*(1.0+eta)*(1.0+zeta),
                            0.125*(1.0-xi) *(1.0+zeta),
                            0.125*(1.0-xi) *(1.0+eta));
      default: return Real3(0.0, 0.0, 0.0);
    }
  }
#endif
};

// ── HEX20 (serendipity hexahedron, 20 nodes) ─────────────────────────────────
// Corner nodes (0-7) use serendipity formula:
//   phi_i = 0.125*(1+sx*xi)*(1+sy*eta)*(1+sz*zeta)*(sx*xi+sy*eta+sz*zeta-2)
// Midside nodes on xi-edges (xi varies, sy,sz fixed):
//   phi = 0.25*(1-xi²)*(1+sy*eta)*(1+sz*zeta)
// Midside nodes on eta-edges (eta varies, sx,sz fixed):
//   phi = 0.25*(1+sx*xi)*(1-eta²)*(1+sz*zeta)
// Midside nodes on zeta-edges (zeta varies, sx,sy fixed):
//   phi = 0.25*(1+sx*xi)*(1+sy*eta)*(1-zeta²)
//
// Node ordering (libMesh):
//   0-7:   same corners as HEX8
//   8:  xi-edge,   sy=-1, sz=-1   (midpoint of edge 0-1)
//   9:  eta-edge,  sx=+1, sz=-1   (midpoint of edge 1-2)
//  10:  xi-edge,   sy=+1, sz=-1   (midpoint of edge 2-3)
//  11:  eta-edge,  sx=-1, sz=-1   (midpoint of edge 3-0)
//  12:  zeta-edge, sx=-1, sy=-1   (midpoint of edge 0-4)
//  13:  zeta-edge, sx=+1, sy=-1   (midpoint of edge 1-5)
//  14:  zeta-edge, sx=+1, sy=+1   (midpoint of edge 2-6)
//  15:  zeta-edge, sx=-1, sy=+1   (midpoint of edge 3-7)
//  16:  xi-edge,   sy=-1, sz=+1   (midpoint of edge 4-5)
//  17:  eta-edge,  sx=+1, sz=+1   (midpoint of edge 5-6)
//  18:  xi-edge,   sy=+1, sz=+1   (midpoint of edge 6-7)
//  19:  eta-edge,  sx=-1, sz=+1   (midpoint of edge 7-4)

template <>
struct FEEvaluator<LagrangeTag, Hex20Tag>
{
  static constexpr unsigned int n_dofs() { return 20; }

#ifdef MOOSE_KOKKOS_SCOPE
  KOKKOS_INLINE_FUNCTION static Real
  shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    switch (i)
    {
      // Corner nodes: phi = 0.125*(1+sx*xi)*(1+sy*eta)*(1+sz*zeta)*(sx*xi+sy*eta+sz*zeta-2)
      case 0:  return 0.125*(1.0-xi)*(1.0-eta)*(1.0-zeta)*(-xi-eta-zeta-2.0);
      case 1:  return 0.125*(1.0+xi)*(1.0-eta)*(1.0-zeta)*( xi-eta-zeta-2.0);
      case 2:  return 0.125*(1.0+xi)*(1.0+eta)*(1.0-zeta)*( xi+eta-zeta-2.0);
      case 3:  return 0.125*(1.0-xi)*(1.0+eta)*(1.0-zeta)*(-xi+eta-zeta-2.0);
      case 4:  return 0.125*(1.0-xi)*(1.0-eta)*(1.0+zeta)*(-xi-eta+zeta-2.0);
      case 5:  return 0.125*(1.0+xi)*(1.0-eta)*(1.0+zeta)*( xi-eta+zeta-2.0);
      case 6:  return 0.125*(1.0+xi)*(1.0+eta)*(1.0+zeta)*( xi+eta+zeta-2.0);
      case 7:  return 0.125*(1.0-xi)*(1.0+eta)*(1.0+zeta)*(-xi+eta+zeta-2.0);
      // Midside nodes on xi-edges: phi = 0.25*(1-xi²)*(1+sy*eta)*(1+sz*zeta)
      case 8:  return 0.25*(1.0-xi*xi)*(1.0-eta)*(1.0-zeta); // sy=-1, sz=-1
      case 10: return 0.25*(1.0-xi*xi)*(1.0+eta)*(1.0-zeta); // sy=+1, sz=-1
      case 16: return 0.25*(1.0-xi*xi)*(1.0-eta)*(1.0+zeta); // sy=-1, sz=+1
      case 18: return 0.25*(1.0-xi*xi)*(1.0+eta)*(1.0+zeta); // sy=+1, sz=+1
      // Midside nodes on eta-edges: phi = 0.25*(1+sx*xi)*(1-eta²)*(1+sz*zeta)
      case 9:  return 0.25*(1.0+xi)*(1.0-eta*eta)*(1.0-zeta); // sx=+1, sz=-1
      case 11: return 0.25*(1.0-xi)*(1.0-eta*eta)*(1.0-zeta); // sx=-1, sz=-1
      case 17: return 0.25*(1.0+xi)*(1.0-eta*eta)*(1.0+zeta); // sx=+1, sz=+1
      case 19: return 0.25*(1.0-xi)*(1.0-eta*eta)*(1.0+zeta); // sx=-1, sz=+1
      // Midside nodes on zeta-edges: phi = 0.25*(1+sx*xi)*(1+sy*eta)*(1-zeta²)
      case 12: return 0.25*(1.0-xi)*(1.0-eta)*(1.0-zeta*zeta); // sx=-1, sy=-1
      case 13: return 0.25*(1.0+xi)*(1.0-eta)*(1.0-zeta*zeta); // sx=+1, sy=-1
      case 14: return 0.25*(1.0+xi)*(1.0+eta)*(1.0-zeta*zeta); // sx=+1, sy=+1
      case 15: return 0.25*(1.0-xi)*(1.0+eta)*(1.0-zeta*zeta); // sx=-1, sy=+1
      default: return 0.0;
    }
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    switch (i)
    {
      // Corner gradient formulas:
      //   d/dxi   = 0.125*sx*(1+sy*eta)*(1+sz*zeta)*(2*sx*xi+sy*eta+sz*zeta-1)
      //   d/deta  = 0.125*sy*(1+sx*xi)*(1+sz*zeta)*(sx*xi+2*sy*eta+sz*zeta-1)
      //   d/dzeta = 0.125*sz*(1+sx*xi)*(1+sy*eta)*(sx*xi+sy*eta+2*sz*zeta-1)

      // Node 0: sx=-1, sy=-1, sz=-1
      case 0: return Real3(
        -0.125*(1.0-eta)*(1.0-zeta)*(-2.0*xi-eta-zeta-1.0),
        -0.125*(1.0-xi) *(1.0-zeta)*(-xi-2.0*eta-zeta-1.0),
        -0.125*(1.0-xi) *(1.0-eta) *(-xi-eta-2.0*zeta-1.0));
      // Node 1: sx=+1, sy=-1, sz=-1
      case 1: return Real3(
         0.125*(1.0-eta)*(1.0-zeta)*(2.0*xi-eta-zeta-1.0),
        -0.125*(1.0+xi) *(1.0-zeta)*(xi-2.0*eta-zeta-1.0),
        -0.125*(1.0+xi) *(1.0-eta) *(xi-eta-2.0*zeta-1.0));
      // Node 2: sx=+1, sy=+1, sz=-1
      case 2: return Real3(
         0.125*(1.0+eta)*(1.0-zeta)*(2.0*xi+eta-zeta-1.0),
         0.125*(1.0+xi) *(1.0-zeta)*(xi+2.0*eta-zeta-1.0),
        -0.125*(1.0+xi) *(1.0+eta) *(xi+eta-2.0*zeta-1.0));
      // Node 3: sx=-1, sy=+1, sz=-1
      case 3: return Real3(
        -0.125*(1.0+eta)*(1.0-zeta)*(-2.0*xi+eta-zeta-1.0),
         0.125*(1.0-xi) *(1.0-zeta)*(-xi+2.0*eta-zeta-1.0),
        -0.125*(1.0-xi) *(1.0+eta) *(-xi+eta-2.0*zeta-1.0));
      // Node 4: sx=-1, sy=-1, sz=+1
      case 4: return Real3(
        -0.125*(1.0-eta)*(1.0+zeta)*(-2.0*xi-eta+zeta-1.0),
        -0.125*(1.0-xi) *(1.0+zeta)*(-xi-2.0*eta+zeta-1.0),
         0.125*(1.0-xi) *(1.0-eta) *(-xi-eta+2.0*zeta-1.0));
      // Node 5: sx=+1, sy=-1, sz=+1
      case 5: return Real3(
         0.125*(1.0-eta)*(1.0+zeta)*(2.0*xi-eta+zeta-1.0),
        -0.125*(1.0+xi) *(1.0+zeta)*(xi-2.0*eta+zeta-1.0),
         0.125*(1.0+xi) *(1.0-eta) *(xi-eta+2.0*zeta-1.0));
      // Node 6: sx=+1, sy=+1, sz=+1
      case 6: return Real3(
         0.125*(1.0+eta)*(1.0+zeta)*(2.0*xi+eta+zeta-1.0),
         0.125*(1.0+xi) *(1.0+zeta)*(xi+2.0*eta+zeta-1.0),
         0.125*(1.0+xi) *(1.0+eta) *(xi+eta+2.0*zeta-1.0));
      // Node 7: sx=-1, sy=+1, sz=+1
      case 7: return Real3(
        -0.125*(1.0+eta)*(1.0+zeta)*(-2.0*xi+eta+zeta-1.0),
         0.125*(1.0-xi) *(1.0+zeta)*(-xi+2.0*eta+zeta-1.0),
         0.125*(1.0-xi) *(1.0+eta) *(-xi+eta+2.0*zeta-1.0));

      // Xi-edge midside gradients (1-xi²) factor:
      //   d/dxi = -2*xi*(...),  d/deta = 0.25*(1-xi²)*sy*(...),  d/dzeta = 0.25*(1-xi²)*(..)*sz
      case 8:  return Real3(-0.5*xi*(1.0-eta)*(1.0-zeta),
                            -0.25*(1.0-xi*xi)*(1.0-zeta),
                            -0.25*(1.0-xi*xi)*(1.0-eta));
      case 10: return Real3(-0.5*xi*(1.0+eta)*(1.0-zeta),
                             0.25*(1.0-xi*xi)*(1.0-zeta),
                            -0.25*(1.0-xi*xi)*(1.0+eta));
      case 16: return Real3(-0.5*xi*(1.0-eta)*(1.0+zeta),
                            -0.25*(1.0-xi*xi)*(1.0+zeta),
                             0.25*(1.0-xi*xi)*(1.0-eta));
      case 18: return Real3(-0.5*xi*(1.0+eta)*(1.0+zeta),
                             0.25*(1.0-xi*xi)*(1.0+zeta),
                             0.25*(1.0-xi*xi)*(1.0+eta));

      // Eta-edge midside gradients:
      case 9:  return Real3( 0.25*(1.0-eta*eta)*(1.0-zeta),
                            -0.5*eta*(1.0+xi)*(1.0-zeta),
                            -0.25*(1.0+xi)*(1.0-eta*eta));
      case 11: return Real3(-0.25*(1.0-eta*eta)*(1.0-zeta),
                            -0.5*eta*(1.0-xi)*(1.0-zeta),
                            -0.25*(1.0-xi)*(1.0-eta*eta));
      case 17: return Real3( 0.25*(1.0-eta*eta)*(1.0+zeta),
                            -0.5*eta*(1.0+xi)*(1.0+zeta),
                             0.25*(1.0+xi)*(1.0-eta*eta));
      case 19: return Real3(-0.25*(1.0-eta*eta)*(1.0+zeta),
                            -0.5*eta*(1.0-xi)*(1.0+zeta),
                             0.25*(1.0-xi)*(1.0-eta*eta));

      // Zeta-edge midside gradients:
      case 12: return Real3(-0.25*(1.0-eta)*(1.0-zeta*zeta),
                            -0.25*(1.0-xi)*(1.0-zeta*zeta),
                            -0.5*zeta*(1.0-xi)*(1.0-eta));
      case 13: return Real3( 0.25*(1.0-eta)*(1.0-zeta*zeta),
                            -0.25*(1.0+xi)*(1.0-zeta*zeta),
                            -0.5*zeta*(1.0+xi)*(1.0-eta));
      case 14: return Real3( 0.25*(1.0+eta)*(1.0-zeta*zeta),
                             0.25*(1.0+xi)*(1.0-zeta*zeta),
                            -0.5*zeta*(1.0+xi)*(1.0+eta));
      case 15: return Real3(-0.25*(1.0+eta)*(1.0-zeta*zeta),
                             0.25*(1.0-xi)*(1.0-zeta*zeta),
                            -0.5*zeta*(1.0-xi)*(1.0+eta));

      default: return Real3(0.0, 0.0, 0.0);
    }
  }
#endif
};

// ── HEX27 (triquadratic hexahedron, 27 nodes) ─────────────────────────────────
// Tensor product of three EDGE3 bases.
// libMesh node-to-1D-index mapping (same non-sequential ordering as EDGE3):
//   L_0(t) = 0.5*t*(t-1)   dL_0/dt = t - 0.5
//   L_1(t) = 0.5*t*(t+1)   dL_1/dt = t + 0.5
//   L_2(t) = 1 - t²        dL_2/dt = -2*t
//
// Index tables (from libMesh fe_lagrange_shape_3D.C):
//   i0[] = {0,1,1,0, 0,1,1,0, 2,1,2,0, 0,1,1,0, 2,1,2,0, 2,2,1,2,0,2,2}
//   i1[] = {0,0,1,1, 0,0,1,1, 0,2,1,2, 0,0,1,1, 0,2,1,2, 2,0,2,1,2,2,2}
//   i2[] = {0,0,0,0, 1,1,1,1, 0,0,0,0, 2,2,2,2, 1,1,1,1, 0,2,2,2,2,1,2}

template <>
struct FEEvaluator<LagrangeTag, Hex27Tag>
{
  static constexpr unsigned int n_dofs() { return 27; }

#ifdef MOOSE_KOKKOS_SCOPE
  // Helper: 1D Lagrange basis value (same as QUAD9)
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
  shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    static const unsigned int i0[] =
      {0,1,1,0, 0,1,1,0, 2,1,2,0, 0,1,1,0, 2,1,2,0, 2,2,1,2,0,2,2};
    static const unsigned int i1[] =
      {0,0,1,1, 0,0,1,1, 0,2,1,2, 0,0,1,1, 0,2,1,2, 2,0,2,1,2,2,2};
    static const unsigned int i2[] =
      {0,0,0,0, 1,1,1,1, 0,0,0,0, 2,2,2,2, 1,1,1,1, 0,2,2,2,2,1,2};
    return L(i0[i], xi) * L(i1[i], eta) * L(i2[i], zeta);
  }

  KOKKOS_INLINE_FUNCTION static Real3
  grad_shape(unsigned int i, Real xi, Real eta, Real zeta)
  {
    static const unsigned int i0[] =
      {0,1,1,0, 0,1,1,0, 2,1,2,0, 0,1,1,0, 2,1,2,0, 2,2,1,2,0,2,2};
    static const unsigned int i1[] =
      {0,0,1,1, 0,0,1,1, 0,2,1,2, 0,0,1,1, 0,2,1,2, 2,0,2,1,2,2,2};
    static const unsigned int i2[] =
      {0,0,0,0, 1,1,1,1, 0,0,0,0, 2,2,2,2, 1,1,1,1, 0,2,2,2,2,1,2};
    const Real lxi   = L(i0[i], xi);
    const Real leta  = L(i1[i], eta);
    const Real lzeta = L(i2[i], zeta);
    return Real3(dL(i0[i], xi)  * leta  * lzeta,
                 lxi * dL(i1[i], eta)   * lzeta,
                 lxi * leta  * dL(i2[i], zeta));
  }
#endif
};

} // namespace Moose::Kokkos
