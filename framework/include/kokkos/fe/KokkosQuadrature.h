//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <cmath>
#include "KokkosArray.h"
#include "KokkosFETypes.h"
#include "KokkosTypes.h"

namespace Moose::Kokkos
{

#ifdef MOOSE_KOKKOS_SCOPE

// ─────────────────────────────────────────────────────────────────────────────
//  1-D Gauss-Legendre quadrature on [-1, 1]
//
//  Rules for n = 1 … 7 quadrature points.
//  An n-point rule integrates polynomials of degree ≤ 2n-1 exactly.
//
//  Values match the libMesh QGauss implementation (quadrature_gauss_1D.C).
//  All methods carry KOKKOS_INLINE_FUNCTION so they are callable from GPU
//  device code; the switch-based dispatch compiles to fast GPU branch logic.
// ─────────────────────────────────────────────────────────────────────────────

struct GaussLegendre1D
{
  /**
   * Number of 1-D Gauss points required to integrate exactly up to polynomial
   * degree @p alg_order on [-1, 1].  Returns values in [1, 7].
   *
   *   n_points = ceil( (alg_order + 1) / 2 )  (clamped to [1, 7])
   */
  KOKKOS_INLINE_FUNCTION static unsigned int n_points(unsigned int alg_order)
  {
    const unsigned int n = (alg_order + 2u) / 2u; // == ceil((alg_order+1)/2)
    return (n < 1u) ? 1u : (n > 7u ? 7u : n);
  }

  /**
   * The i-th Gauss point of the n-point rule on [-1, 1].
   * Points are in ascending order: x[0] < x[1] < … < x[n-1].
   */
  KOKKOS_INLINE_FUNCTION static Real point(unsigned int n, unsigned int i)
  {
    switch (n)
    {
      case 1:
        // 1-point rule: exact for degree 1
        return 0.0;

      case 2:
        // 2-point rule: exact for degree 3 (±1/√3)
        switch (i)
        {
          case 0: return -5.7735026918962576450914878050196e-01;
          case 1: return  5.7735026918962576450914878050196e-01;
          default: return 0.0;
        }

      case 3:
        // 3-point rule: exact for degree 5 (0, ±√(3/5))
        switch (i)
        {
          case 0: return -7.7459666924148337703585307995648e-01;
          case 1: return  0.0;
          case 2: return  7.7459666924148337703585307995648e-01;
          default: return 0.0;
        }

      case 4:
        // 4-point rule: exact for degree 7
        switch (i)
        {
          case 0: return -8.6113631159405257522394648889281e-01;
          case 1: return -3.3998104358485626480266575910324e-01;
          case 2: return  3.3998104358485626480266575910324e-01;
          case 3: return  8.6113631159405257522394648889281e-01;
          default: return 0.0;
        }

      case 5:
        // 5-point rule: exact for degree 9
        switch (i)
        {
          case 0: return -9.0617984593866399279762687829939e-01;
          case 1: return -5.3846931010568309103631442070021e-01;
          case 2: return  0.0;
          case 3: return  5.3846931010568309103631442070021e-01;
          case 4: return  9.0617984593866399279762687829939e-01;
          default: return 0.0;
        }

      case 6:
        // 6-point rule: exact for degree 11
        switch (i)
        {
          case 0: return -9.3246951420315202781230155449399e-01;
          case 1: return -6.6120938646626451366139959501991e-01;
          case 2: return -2.3861918608319690863050172168071e-01;
          case 3: return  2.3861918608319690863050172168071e-01;
          case 4: return  6.6120938646626451366139959501991e-01;
          case 5: return  9.3246951420315202781230155449399e-01;
          default: return 0.0;
        }

      case 7:
        // 7-point rule: exact for degree 13
        switch (i)
        {
          case 0: return -9.4910791234275852452618968404785e-01;
          case 1: return -7.4153118559939443986386477328079e-01;
          case 2: return -4.0584515137739716690660641207696e-01;
          case 3: return  0.0;
          case 4: return  4.0584515137739716690660641207696e-01;
          case 5: return  7.4153118559939443986386477328079e-01;
          case 6: return  9.4910791234275852452618968404785e-01;
          default: return 0.0;
        }

      default:
        return 0.0;
    }
  }

  /**
   * The weight of the i-th quadrature point of the n-point rule.
   * Weights are positive and sum to 2 (the length of [-1, 1]).
   */
  KOKKOS_INLINE_FUNCTION static Real weight(unsigned int n, unsigned int i)
  {
    switch (n)
    {
      case 1:
        return 2.0;

      case 2:
        return 1.0;

      case 3:
        // weights: 5/9, 8/9, 5/9
        switch (i)
        {
          case 0: case 2: return 5.5555555555555555555555555555556e-01;
          case 1:         return 8.8888888888888888888888888888889e-01;
          default: return 0.0;
        }

      case 4:
        switch (i)
        {
          case 0: case 3: return 3.4785484513745385737306394922200e-01;
          case 1: case 2: return 6.5214515486254614262693605077800e-01;
          default: return 0.0;
        }

      case 5:
        switch (i)
        {
          case 0: case 4: return 2.3692688505618908751426404071992e-01;
          case 1: case 3: return 4.7862867049936646804129151483564e-01;
          case 2:         return 5.6888888888888888888888888888889e-01;
          default: return 0.0;
        }

      case 6:
        switch (i)
        {
          case 0: case 5: return 1.7132449237917034504029614217273e-01;
          case 1: case 4: return 3.6076157304813860756983351383772e-01;
          case 2: case 3: return 4.6791393457269104738987034398955e-01;
          default: return 0.0;
        }

      case 7:
        switch (i)
        {
          case 0: case 6: return 1.2948496616886969327061143267908e-01;
          case 1: case 5: return 2.7970539148927666790146777142378e-01;
          case 2: case 4: return 3.8183005050511894495036977548898e-01;
          case 3:         return 4.1795918367346938775510204081633e-01;
          default: return 0.0;
        }

      default:
        return 0.0;
    }
  }
};

// ─────────────────────────────────────────────────────────────────────────────
//  fillQuadrature — CPU-side quadrature dispatcher
//
//  Creates and fills @p qpts and @p weights for the given element topology and
//  polynomial order.  Matches the point/weight layout produced by libMesh's
//  QGauss::init() with allow_rules_with_negative_weights = true (the default).
//
//  Coordinate conventions (same as libMesh):
//    EDGE2/3  : xi  in [-1,1], stored in Real3.v[0]
//    QUAD4/8/9: xi,eta in [-1,1]^2, tensor product (outer loop eta, inner xi)
//    HEX8/20/27: xi,eta,zeta in [-1,1]^3, tensor product
//    TRI3/6   : (x,y) on unit triangle {(0,0),(1,0),(0,1)}, Dunavant-type rules
//    TET4/10  : (x,y,z) on unit tet {(0,0,0)…(0,0,1)}, Keast/Walkington rules
// ─────────────────────────────────────────────────────────────────────────────

static inline void
fillQuadrature(FEElemTopology topo,
               unsigned int order,
               Array<Real3> & qpts,
               Array<Real> & weights)
{
  switch (topo)
  {
    // ── 1-D elements ────────────────────────────────────────────────────────
    case FEElemTopology::EDGE2:
    case FEElemTopology::EDGE3:
    {
      const unsigned int n = GaussLegendre1D::n_points(order);
      qpts.create(n);
      weights.create(n);
      for (unsigned int i = 0; i < n; ++i)
      {
        qpts[i]    = Real3(GaussLegendre1D::point(n, i), 0.0, 0.0);
        weights[i] = GaussLegendre1D::weight(n, i);
      }
      return;
    }

    // ── 2-D quadrilateral elements (tensor product) ──────────────────────────
    case FEElemTopology::QUAD4:
    case FEElemTopology::QUAD8:
    case FEElemTopology::QUAD9:
    {
      const unsigned int n  = GaussLegendre1D::n_points(order);
      const unsigned int n2 = n * n;
      qpts.create(n2);
      weights.create(n2);
      unsigned int q = 0;
      for (unsigned int j = 0; j < n; ++j)            // eta loop (outer)
        for (unsigned int i = 0; i < n; ++i)          // xi loop (inner)
        {
          qpts[q]    = Real3(GaussLegendre1D::point(n, i),
                             GaussLegendre1D::point(n, j),
                             0.0);
          weights[q] = GaussLegendre1D::weight(n, i) * GaussLegendre1D::weight(n, j);
          ++q;
        }
      return;
    }

    // ── 3-D hexahedral elements (tensor product) ─────────────────────────────
    case FEElemTopology::HEX8:
    case FEElemTopology::HEX20:
    case FEElemTopology::HEX27:
    {
      const unsigned int n  = GaussLegendre1D::n_points(order);
      const unsigned int n3 = n * n * n;
      qpts.create(n3);
      weights.create(n3);
      unsigned int q = 0;
      for (unsigned int k = 0; k < n; ++k)            // zeta loop (outer)
        for (unsigned int j = 0; j < n; ++j)          // eta loop
          for (unsigned int i = 0; i < n; ++i)        // xi loop (inner)
          {
            qpts[q]    = Real3(GaussLegendre1D::point(n, i),
                               GaussLegendre1D::point(n, j),
                               GaussLegendre1D::point(n, k));
            weights[q] = GaussLegendre1D::weight(n, i) *
                         GaussLegendre1D::weight(n, j) *
                         GaussLegendre1D::weight(n, k);
            ++q;
          }
      return;
    }

    // ── 2-D triangular elements ───────────────────────────────────────────────
    //  Rules from libMesh quadrature_gauss_2D.C (QGauss::init_2D, TRI cases).
    //  Coordinates are on the unit triangle {(0,0),(1,0),(0,1)}.
    //  Weights sum to 0.5 (the area of the unit triangle).
    case FEElemTopology::TRI3:
    case FEElemTopology::TRI6:
    {
      switch (order)
      {
        // ── order 0 or 1: 1-point centroid rule ────────────────────────────
        case 0:
        case 1:
        {
          qpts.create(1);
          weights.create(1);
          qpts[0]    = Real3(1.0 / 3.0, 1.0 / 3.0, 0.0);
          weights[0] = 0.5;
          return;
        }

        // ── order 2: 3-point rule, exact for quadratics ─────────────────────
        //  libMesh: (2/3, 1/6), (1/6, 2/3), (1/6, 1/6), w = 1/6 each
        case 2:
        {
          qpts.create(3);
          weights.create(3);
          qpts[0]    = Real3(2.0 / 3.0, 1.0 / 6.0, 0.0);
          qpts[1]    = Real3(1.0 / 6.0, 2.0 / 3.0, 0.0);
          qpts[2]    = Real3(1.0 / 6.0, 1.0 / 6.0, 0.0);
          weights[0] = 1.0 / 6.0;
          weights[1] = 1.0 / 6.0;
          weights[2] = 1.0 / 6.0;
          return;
        }

        // ── order 3: 4-point conical product rule, exact for cubics ─────────
        //  libMesh: hard-coded tensor product of Gauss/Jacobi rules.
        case 3:
        {
          qpts.create(4);
          weights.create(4);
          qpts[0] = Real3(1.5505102572168219018027159252941e-01,
                          1.7855872826361642311703513337422e-01,
                          0.0);
          qpts[1] = Real3(6.4494897427831780981972840747059e-01,
                          7.5031110222608118177475598324603e-02,
                          0.0);
          qpts[2] = Real3(1.5505102572168219018027159252941e-01,
                          6.6639024601470138670269327409637e-01,
                          0.0);
          qpts[3] = Real3(6.4494897427831780981972840747059e-01,
                          2.8001991549907407200279599420481e-01,
                          0.0);
          weights[0] = 1.5902069087198858469718450103758e-01;
          weights[1] = 9.0979309128011415302815498962418e-02;
          weights[2] = weights[0];
          weights[3] = weights[1];
          return;
        }

        // ── order 4: 6-point Dunavant rule ──────────────────────────────────
        //  dunavant_rule2 with permutation_ids = {3, 3}
        //  wts = {0.11169..., 0.05498...}
        //  a   = {0.44595..., 0.09158...}
        case 4:
        {
          // Group 1: a1 = 0.44595..., b1 = 1 - 2*a1
          constexpr Real a1  = 4.4594849091596488631832925388305e-01;
          constexpr Real b1  = 1.0 - 2.0 * a1; // 0.10810301816807022736...
          constexpr Real w1  = 1.1169079483900573284750350421656e-01;
          // Group 2: a2 = 0.09158..., b2 = 1 - 2*a2
          constexpr Real a2  = 9.1576213509770743459571463402202e-02;
          constexpr Real b2  = 1.0 - 2.0 * a2; // 0.81684757298045851308...
          constexpr Real w2  = 5.4975871827660933819163162450105e-02;

          qpts.create(6);
          weights.create(6);
          // permutation (a,a), (a,1-2a), (1-2a,a)
          qpts[0] = Real3(a1, a1, 0.0); qpts[1] = Real3(a1, b1, 0.0); qpts[2] = Real3(b1, a1, 0.0);
          qpts[3] = Real3(a2, a2, 0.0); qpts[4] = Real3(a2, b2, 0.0); qpts[5] = Real3(b2, a2, 0.0);
          for (unsigned int i = 0; i < 3; ++i) weights[i] = w1;
          for (unsigned int i = 3; i < 6; ++i) weights[i] = w2;
          return;
        }

        // ── order 5: 7-point Dunavant/Walkington rule ────────────────────────
        //  permutation_ids = {1, 3, 3}
        //  wts = {9/80, 31/480 + √15/2400, 31/480 − √15/2400}
        //  a   = {0 (centroid), 2/7 + √15/21, 2/7 − √15/21}
        case 5:
        {
          const Real sq15 = std::sqrt(static_cast<Real>(15));
          const Real w0   = Real(9) / Real(80);
          const Real w1   = Real(31) / Real(480) + sq15 / Real(2400);
          const Real w2   = Real(31) / Real(480) - sq15 / Real(2400);
          const Real a1   = Real(2) / Real(7) + sq15 / Real(21);
          const Real a2   = Real(2) / Real(7) - sq15 / Real(21);
          const Real b1   = 1.0 - 2.0 * a1;
          const Real b2   = 1.0 - 2.0 * a2;

          qpts.create(7);
          weights.create(7);
          // centroid
          qpts[0]    = Real3(1.0 / 3.0, 1.0 / 3.0, 0.0);
          weights[0] = w0;
          // group 1: (a1,a1), (a1,b1), (b1,a1)
          qpts[1] = Real3(a1, a1, 0.0); qpts[2] = Real3(a1, b1, 0.0); qpts[3] = Real3(b1, a1, 0.0);
          weights[1] = weights[2] = weights[3] = w1;
          // group 2: (a2,a2), (a2,b2), (b2,a2)
          qpts[4] = Real3(a2, a2, 0.0); qpts[5] = Real3(a2, b2, 0.0); qpts[6] = Real3(b2, a2, 0.0);
          weights[4] = weights[5] = weights[6] = w2;
          return;
        }

        // ── order 6: 12-point Dunavant rule ─────────────────────────────────
        //  permutation_ids = {3, 3, 6}
        //  From Lyness & Jespersen; extra precision from Zhang, Cui & Liu 2009.
        case 6:
        {
          // Group 1: S21 permutation (a, a, 1-2a)
          constexpr Real a1 = 2.4928674517091042129163855310702e-01;
          constexpr Real b1 = 1.0 - 2.0 * a1; // 0.50142650965817915416...
          constexpr Real w1 = 5.8393137863189683012644805692790e-02;
          // Group 2: S21 permutation
          constexpr Real a2 = 6.3089014491502228340331602870819e-02;
          constexpr Real b2 = 1.0 - 2.0 * a2; // 0.87382197101699554332...
          constexpr Real w2 = 2.5422453185103408460468404553434e-02;
          // Group 3: S111 permutation (a, b, 1-a-b)
          constexpr Real a3 = 3.1035245103378440541660773395655e-01;
          constexpr Real b3 = 6.3650249912139864723014259441205e-01;
          constexpr Real c3 = 1.0 - a3 - b3; // 0.05314504984481694744...
          constexpr Real w3 = 4.1425537809186787596776728210221e-02;

          qpts.create(12);
          weights.create(12);
          // Group 1
          qpts[0] = Real3(a1, a1, 0.0); qpts[1] = Real3(a1, b1, 0.0); qpts[2] = Real3(b1, a1, 0.0);
          for (unsigned int i = 0; i < 3; ++i) weights[i] = w1;
          // Group 2
          qpts[3] = Real3(a2, a2, 0.0); qpts[4] = Real3(a2, b2, 0.0); qpts[5] = Real3(b2, a2, 0.0);
          for (unsigned int i = 3; i < 6; ++i) weights[i] = w2;
          // Group 3: (a,b), (b,a), (a,c), (c,a), (b,c), (c,b)
          qpts[6]  = Real3(a3, b3, 0.0); qpts[7]  = Real3(b3, a3, 0.0);
          qpts[8]  = Real3(a3, c3, 0.0); qpts[9]  = Real3(c3, a3, 0.0);
          qpts[10] = Real3(b3, c3, 0.0); qpts[11] = Real3(c3, b3, 0.0);
          for (unsigned int i = 6; i < 12; ++i) weights[i] = w3;
          return;
        }

        // ── order 7 (and higher): 12-point Ro3-invariant rule (Gatermann) ───
        //  Cyclic permutations (z1,z2) → {(z1,z2), (z3,z1), (z2,z3)} where z3=1-z1-z2.
        //  Exact for degree 7; minimal for 12-point triangle rules.
        default:
        {
          // Raw data: {z1, z2, weight} for each group of 3 cyclic points
          constexpr Real rd[4][3] = {
            {6.2382265094402118173683000996350e-02,
             6.7517867073916085442557131050869e-02,
             2.6517028157436251428754180460739e-02},
            {5.5225456656926611737479190275645e-02,
             3.2150249385198182266630784919920e-01,
             4.3881408714446055036769903139288e-02},
            {3.4324302945097146469630642483938e-02,
             6.6094919618673565761198031019780e-01,
             2.8775042784981585738445496900219e-02},
            {5.1584233435359177925746338682643e-01,
             2.7771616697639178256958187139372e-01,
             6.7493187009802774462697086166421e-02}
          };

          qpts.create(12);
          weights.create(12);
          for (unsigned int row = 0, q = 0; row < 4; ++row)
          {
            const Real z1 = rd[row][0], z2 = rd[row][1], z3 = 1.0 - z1 - z2;
            const Real wt = rd[row][2];
            qpts[q] = Real3(z1, z2, 0.0); weights[q++] = wt;
            qpts[q] = Real3(z3, z1, 0.0); weights[q++] = wt;
            qpts[q] = Real3(z2, z3, 0.0); weights[q++] = wt;
          }
          return;
        }
      } // switch (order) for TRI
    } // TRI case

    // ── 3-D tetrahedral elements ───────────────────────────────────────────────
    //  Rules from libMesh quadrature_gauss_3D.C (QGauss::init_3D, TET cases).
    //  Coordinates are on the unit tet {(0,0,0),(1,0,0),(0,1,0),(0,0,1)}.
    //  Weights sum to 1/6 (the volume of the unit tetrahedron).
    case FEElemTopology::TET4:
    case FEElemTopology::TET10:
    {
      switch (order)
      {
        // ── order 0 or 1: 1-point centroid rule ────────────────────────────
        case 0:
        case 1:
        {
          qpts.create(1);
          weights.create(1);
          qpts[0]    = Real3(0.25, 0.25, 0.25);
          weights[0] = 1.0 / 6.0;
          return;
        }

        // ── order 2: 4-point rule, exact for quadratics ─────────────────────
        //  b = 0.25*(1 - 1/√5),  a = 1 - 3b.
        //  4-permutation: (a,b,b), (b,a,b), (b,b,a), (b,b,b).
        case 2:
        {
          const Real b = 0.25 * (1.0 - 1.0 / std::sqrt(static_cast<Real>(5)));
          const Real a = 1.0 - 3.0 * b;
          qpts.create(4);
          weights.create(4);
          qpts[0] = Real3(a, b, b);
          qpts[1] = Real3(b, a, b);
          qpts[2] = Real3(b, b, a);
          qpts[3] = Real3(b, b, b);
          for (unsigned int i = 0; i < 4; ++i)
            weights[i] = 1.0 / 24.0;
          return;
        }

        // ── order 3: 5-point rule (negative weight), exact for cubics ───────
        //  Flaherty's rule.  One negative central weight.
        //  Falls through to order 4 when !allow_rules_with_negative_weights
        //  (here we provide the standard negative-weight version).
        case 3:
        {
          qpts.create(5);
          weights.create(5);
          qpts[0] = Real3(0.25,          0.25,          0.25);
          qpts[1] = Real3(0.5,           1.0 / 6.0,     1.0 / 6.0);
          qpts[2] = Real3(1.0 / 6.0,     0.5,           1.0 / 6.0);
          qpts[3] = Real3(1.0 / 6.0,     1.0 / 6.0,     0.5);
          qpts[4] = Real3(1.0 / 6.0,     1.0 / 6.0,     1.0 / 6.0);
          weights[0] = -2.0 / 15.0;
          weights[1] = weights[2] = weights[3] = weights[4] = 0.075;
          return;
        }

        // ── order 4: 11-point Keast rule (negative weight) ───────────────────
        //  Patrick Keast, CMAME 55 (1986) pp. 339-348.
        //  keast_rule format: {a, b, 0, w} = 4-perm; {a, 0, b, w} = 6-perm.
        case 4:
        {
          // Row 0: {0.25, 0, 0, -0.01316}: 1-perm  → (0.25, 0.25, 0.25)
          // Row 1: {0.7857, 0.07143, 0, 0.00762}: 4-perm → (a,b,b),(b,a,b),(b,b,a),(b,b,b)
          // Row 2: {0.3994, 0, 0.1006, 0.02489}: 6-perm → (a,a,b),(a,b,b),(b,b,a),(b,a,b),(b,a,a),(a,b,a)
          constexpr Real a1 = 2.5e-01,                  w0 = -1.31555555555555556e-02;
          constexpr Real a2 = 7.85714285714285714e-01,  b2 = 7.14285714285714285e-02, w1 =  7.62222222222222222e-03;
          constexpr Real a3 = 3.99403576166799219e-01,  b3 = 1.00596423833200785e-01, w2 =  2.48888888888888889e-02;

          qpts.create(11);
          weights.create(11);
          unsigned int q = 0;
          // 1-perm
          qpts[q] = Real3(a1, a1, a1); weights[q++] = w0;
          // 4-perm: (a,b,b), (b,a,b), (b,b,a), (b,b,b)
          qpts[q] = Real3(a2, b2, b2); weights[q++] = w1;
          qpts[q] = Real3(b2, a2, b2); weights[q++] = w1;
          qpts[q] = Real3(b2, b2, a2); weights[q++] = w1;
          qpts[q] = Real3(b2, b2, b2); weights[q++] = w1;
          // 6-perm: (a,a,b),(a,b,b),(b,b,a),(b,a,b),(b,a,a),(a,b,a)
          qpts[q] = Real3(a3, a3, b3); weights[q++] = w2;
          qpts[q] = Real3(a3, b3, b3); weights[q++] = w2;
          qpts[q] = Real3(b3, b3, a3); weights[q++] = w2;
          qpts[q] = Real3(b3, a3, b3); weights[q++] = w2;
          qpts[q] = Real3(b3, a3, a3); weights[q++] = w2;
          qpts[q] = Real3(a3, b3, a3); weights[q++] = w2;
          return;
        }

        // ── order 5: 14-point Walkington rule, exact for 5th degree ─────────
        //  "Quadrature on Simplices of Arbitrary Dimension", Walkington.
        //  Two groups of 4 symmetric points + one group of 6 mid-edge points.
        case 5:
        {
          const Real af[3] = {3.1088591926330060980e-01,
                              9.2735250310891226402e-02,
                              4.5503704125649649492e-02};
          const Real wf[3] = {1.8781320953002641800e-02,
                              1.2248840519393658257e-02,
                              7.0910034628469110730e-03};

          qpts.create(14);
          weights.create(14);
          unsigned int q = 0;
          // Two groups of 4: (a,a,a), (a,b,a), (b,a,a), (a,a,b) where b=1-3a
          for (unsigned int g = 0; g < 2; ++g)
          {
            const Real ag = af[g], bg = 1.0 - 3.0 * ag;
            qpts[q] = Real3(ag, ag, ag); weights[q++] = wf[g];
            qpts[q] = Real3(ag, bg, ag); weights[q++] = wf[g];
            qpts[q] = Real3(bg, ag, ag); weights[q++] = wf[g];
            qpts[q] = Real3(ag, ag, bg); weights[q++] = wf[g];
          }
          // One group of 6: b=0.5*(1-2*a[2]); (b,b,a),(b,a,a),(a,a,b),(a,b,a),(b,a,b),(a,b,b)
          {
            const Real a2 = af[2], b2 = 0.5 * (1.0 - 2.0 * a2);
            qpts[q]  = Real3(b2, b2, a2); weights[q++] = wf[2];
            qpts[q]  = Real3(b2, a2, a2); weights[q++] = wf[2];
            qpts[q]  = Real3(a2, a2, b2); weights[q++] = wf[2];
            qpts[q]  = Real3(a2, b2, a2); weights[q++] = wf[2];
            qpts[q]  = Real3(b2, a2, b2); weights[q++] = wf[2];
            qpts[q]  = Real3(a2, b2, b2); weights[q++] = wf[2];
          }
          return;
        }

        // ── order 6 (and higher): 24-point Keast rule ────────────────────────
        //  Patrick Keast, CMAME 55 (1986) pp. 339-348; Table KEAST6.
        //  Three 4-permutation groups + one 12-permutation group.
        //  keast_rule format: {a,b,0,w}=4-perm; {a,b,c,w}=12-perm.
        default:
        {
          // Raw data matching keast_rule() expansion
          //   {a, b, 0, w}  → 4-perm: (a,b,b),(b,a,b),(b,b,a),(b,b,b)
          //   {a, b, c, w}  → 12-perm (a≠b≠c≠0)
          constexpr Real ar  = 3.56191386222544953e-01, br  = 2.14602871259151684e-01, wr  = 6.65379170969464506e-03;
          constexpr Real ar2 = 8.77978124396165982e-01, br2 = 4.06739585346113397e-02, wr2 = 1.67953517588677620e-03;
          constexpr Real ar3 = 3.29863295731730594e-02, br3 = 3.22337890142275646e-01, wr3 = 9.22619692394239843e-03;
          // 12-perm group
          constexpr Real a4  = 6.36610018750175299e-02;
          constexpr Real b4  = 2.69672331458315867e-01;
          constexpr Real c4  = 6.03005664791649076e-01;
          constexpr Real w4  = 8.03571428571428248e-03;

          qpts.create(24);
          weights.create(24);
          unsigned int q = 0;

          // Helper lambda: expand a 4-permutation {a, b, b}
          auto fill4 = [&](Real a, Real b, Real w) {
            qpts[q] = Real3(a, b, b); weights[q++] = w;
            qpts[q] = Real3(b, a, b); weights[q++] = w;
            qpts[q] = Real3(b, b, a); weights[q++] = w;
            qpts[q] = Real3(b, b, b); weights[q++] = w;
          };
          fill4(ar,  br,  wr);   // 4 pts
          fill4(ar2, br2, wr2);  // 4 pts
          fill4(ar3, br3, wr3);  // 4 pts

          // 12-permutation: (a,a,b),(a,a,c),(b,a,a),(c,a,a),(a,b,a),(a,c,a)
          //                 (a,b,c),(a,c,b),(b,a,c),(b,c,a),(c,a,b),(c,b,a)
          qpts[q] = Real3(a4,a4,b4); weights[q++] = w4;
          qpts[q] = Real3(a4,a4,c4); weights[q++] = w4;
          qpts[q] = Real3(b4,a4,a4); weights[q++] = w4;
          qpts[q] = Real3(c4,a4,a4); weights[q++] = w4;
          qpts[q] = Real3(a4,b4,a4); weights[q++] = w4;
          qpts[q] = Real3(a4,c4,a4); weights[q++] = w4;
          qpts[q] = Real3(a4,b4,c4); weights[q++] = w4;
          qpts[q] = Real3(a4,c4,b4); weights[q++] = w4;
          qpts[q] = Real3(b4,a4,c4); weights[q++] = w4;
          qpts[q] = Real3(b4,c4,a4); weights[q++] = w4;
          qpts[q] = Real3(c4,a4,b4); weights[q++] = w4;
          qpts[q] = Real3(c4,b4,a4); weights[q++] = w4;
          return;
        }
      } // switch (order) for TET
    } // TET case

    default:
      break;
  } // switch (topo)
}

#endif // MOOSE_KOKKOS_SCOPE

} // namespace Moose::Kokkos
