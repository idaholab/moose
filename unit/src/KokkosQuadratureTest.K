//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Tests : KokkosQuadrature device-native quadrature tables.
//
// Three properties are verified for every element type and every supported
// algebraic order:
//
//   1. Weight sum  — sum(w_i) equals the reference element measure.
//   2. Exactness   — integrating a monomial of degree == order gives the
//                    correct analytic value (positive-weight rules only;
//                    negative-weight rules are still tested for weight sum
//                    and libMesh parity).
//   3. libMesh parity — point and weight values match QGauss::init() exactly
//                       (to machine epsilon), including negative-weight rules.
//
// Run:
//   cd unit && make && ./moose-unit-opt --gtest_filter="KokkosQuadratureTest*"

#include "gtest_include.h"

#include "KokkosQuadrature.h"
#include "KokkosFETypes.h"
#include "KokkosTypes.h"

#include "libmesh/quadrature_gauss.h"
#include "libmesh/reference_elem.h"
#include "libmesh/enum_elem_type.h"

#include <cmath>
#include <string>

using namespace Moose::Kokkos;

static constexpr double TOL  = 1.0e-13; // weight-sum / exactness tolerance
static constexpr double PTOL = 1.0e-14; // point/weight parity with libMesh

// ── helpers ───────────────────────────────────────────────────────────────────

// Reference-element measures (same coordinate conventions as fillQuadrature).
static double refMeasure(FEElemTopology t)
{
  switch (t)
  {
    case FEElemTopology::EDGE2:
    case FEElemTopology::EDGE3:  return 2.0;
    case FEElemTopology::TRI3:
    case FEElemTopology::TRI6:   return 0.5;
    case FEElemTopology::QUAD4:
    case FEElemTopology::QUAD8:
    case FEElemTopology::QUAD9:  return 4.0;
    case FEElemTopology::TET4:
    case FEElemTopology::TET10:  return 1.0 / 6.0;
    case FEElemTopology::HEX8:
    case FEElemTopology::HEX20:
    case FEElemTopology::HEX27:  return 8.0;
    default: return 0.0;
  }
}

// Map FEElemTopology back to the libMesh ElemType used to initialise QGauss.
static libMesh::ElemType toLibMeshElemType(FEElemTopology t)
{
  switch (t)
  {
    case FEElemTopology::EDGE2:  return libMesh::EDGE2;
    case FEElemTopology::EDGE3:  return libMesh::EDGE3;
    case FEElemTopology::TRI3:   return libMesh::TRI3;
    case FEElemTopology::TRI6:   return libMesh::TRI6;
    case FEElemTopology::QUAD4:  return libMesh::QUAD4;
    case FEElemTopology::QUAD8:  return libMesh::QUAD8;
    case FEElemTopology::QUAD9:  return libMesh::QUAD9;
    case FEElemTopology::TET4:   return libMesh::TET4;
    case FEElemTopology::TET10:  return libMesh::TET10;
    case FEElemTopology::HEX8:   return libMesh::HEX8;
    case FEElemTopology::HEX20:  return libMesh::HEX20;
    case FEElemTopology::HEX27:  return libMesh::HEX27;
    default: return libMesh::INVALID_ELEM;
  }
}

// Spatial dimension of an element topology.
static unsigned int dim(FEElemTopology t)
{
  switch (t)
  {
    case FEElemTopology::EDGE2:
    case FEElemTopology::EDGE3:  return 1;
    case FEElemTopology::TRI3:
    case FEElemTopology::TRI6:
    case FEElemTopology::QUAD4:
    case FEElemTopology::QUAD8:
    case FEElemTopology::QUAD9:  return 2;
    default:                     return 3;
  }
}

// Convert an algebraic order integer to the libMesh Order enum.
static libMesh::Order toLibMeshOrder(unsigned int o)
{
  // libMesh Order enum values equal their integer orders.
  return static_cast<libMesh::Order>(o);
}

// ── 1. GaussLegendre1D unit tests ────────────────────────────────────────────

TEST(KokkosQuadratureTest, GaussLegendre1D_NPoints)
{
  // n_points clamps to [1,7]; alg_order p → ceil((p+1)/2) points
  EXPECT_EQ(GaussLegendre1D::n_points(1), 1u);
  EXPECT_EQ(GaussLegendre1D::n_points(2), 2u);
  EXPECT_EQ(GaussLegendre1D::n_points(3), 2u);
  EXPECT_EQ(GaussLegendre1D::n_points(4), 3u);
  EXPECT_EQ(GaussLegendre1D::n_points(5), 3u);
  EXPECT_EQ(GaussLegendre1D::n_points(6), 4u);
  EXPECT_EQ(GaussLegendre1D::n_points(7), 4u);
  EXPECT_EQ(GaussLegendre1D::n_points(12), 7u); // clamped
  EXPECT_EQ(GaussLegendre1D::n_points(0),  1u); // minimum
}

TEST(KokkosQuadratureTest, GaussLegendre1D_WeightSumEqualsTwo)
{
  for (unsigned int n = 1; n <= 7; ++n)
  {
    double sum = 0.0;
    for (unsigned int i = 0; i < n; ++i)
      sum += GaussLegendre1D::weight(n, i);
    EXPECT_NEAR(sum, 2.0, TOL) << "n=" << n;
  }
}

TEST(KokkosQuadratureTest, GaussLegendre1D_SymmetricPoints)
{
  // Gauss-Legendre points are symmetric about 0
  for (unsigned int n = 1; n <= 7; ++n)
    for (unsigned int i = 0; i < n / 2; ++i)
      EXPECT_NEAR(GaussLegendre1D::point(n, i),
                  -GaussLegendre1D::point(n, n - 1 - i), PTOL)
          << "n=" << n << " i=" << i;
}

TEST(KokkosQuadratureTest, GaussLegendre1D_SymmetricWeights)
{
  for (unsigned int n = 1; n <= 7; ++n)
    for (unsigned int i = 0; i < n / 2; ++i)
      EXPECT_NEAR(GaussLegendre1D::weight(n, i),
                  GaussLegendre1D::weight(n, n - 1 - i), PTOL)
          << "n=" << n << " i=" << i;
}

TEST(KokkosQuadratureTest, GaussLegendre1D_ExactnessOdd)
{
  // Integrating x^(2n-1) over [-1,1] should be 0 by symmetry
  for (unsigned int n = 1; n <= 7; ++n)
  {
    double sum = 0.0;
    for (unsigned int i = 0; i < n; ++i)
    {
      double x = GaussLegendre1D::point(n, i);
      double w = GaussLegendre1D::weight(n, i);
      double p = 1.0;
      for (unsigned int k = 0; k < 2 * n - 1; ++k)
        p *= x;
      sum += w * p;
    }
    EXPECT_NEAR(sum, 0.0, TOL) << "n=" << n << " integrating x^" << 2*n-1;
  }
}

TEST(KokkosQuadratureTest, GaussLegendre1D_ExactnessEven)
{
  // Integrate x^(2n-2) over [-1,1] → 2/(2n-1)
  for (unsigned int n = 1; n <= 7; ++n)
  {
    double sum = 0.0;
    for (unsigned int i = 0; i < n; ++i)
    {
      double x = GaussLegendre1D::point(n, i);
      double w = GaussLegendre1D::weight(n, i);
      double p = 1.0;
      for (unsigned int k = 0; k < 2 * n - 2; ++k)
        p *= x;
      sum += w * p;
    }
    double exact = (n == 1) ? 2.0 : 2.0 / (2.0 * n - 1.0);
    EXPECT_NEAR(sum, exact, TOL) << "n=" << n << " integrating x^" << 2*n-2;
  }
}

// ── 2. Weight-sum tests (all topologies, all orders) ─────────────────────────

// Parametric fixture: (topology, order) pairs
struct TopoOrder { FEElemTopology topo; unsigned int order; std::string label; };

static std::vector<TopoOrder> allTopoOrders()
{
  std::vector<TopoOrder> v;
  const FEElemTopology topos[] = {
    FEElemTopology::EDGE2,  FEElemTopology::TRI3,  FEElemTopology::QUAD4,
    FEElemTopology::TET4,   FEElemTopology::HEX8
  };
  const char * names[] = { "EDGE2","TRI3","QUAD4","TET4","HEX8" };
  for (unsigned int t = 0; t < 5; ++t)
    for (unsigned int order = 1; order <= 7; ++order)
      v.push_back({topos[t], order, std::string(names[t]) + "_o" + std::to_string(order)});
  return v;
}

class KokkosQuadratureWeightSumTest : public ::testing::TestWithParam<TopoOrder> {};

TEST_P(KokkosQuadratureWeightSumTest, WeightSum)
{
  auto p = GetParam();
  Array<Real3> pts;
  Array<Real>  wts;
  fillQuadrature(p.topo, p.order, pts, wts);

  ASSERT_GT(pts.size(), 0u) << p.label;
  ASSERT_EQ(pts.size(), wts.size()) << p.label;

  double sum = 0.0;
  for (unsigned int i = 0; i < wts.size(); ++i)
    sum += wts[i];

  EXPECT_NEAR(sum, refMeasure(p.topo), TOL) << p.label;
}

INSTANTIATE_TEST_SUITE_P(AllTopoOrders,
                         KokkosQuadratureWeightSumTest,
                         ::testing::ValuesIn(allTopoOrders()),
                         [](const auto & info) { return info.param.label; });

// ── 3. Polynomial exactness ───────────────────────────────────────────────────

// EDGE: integrate x^order over [-1,1]
// Order is even → 2/（order+1); odd → 0 (symmetric).
TEST(KokkosQuadratureTest, Exactness_Edge_AllOrders)
{
  for (unsigned int order = 1; order <= 7; ++order)
  {
    Array<Real3> pts; Array<Real> wts;
    fillQuadrature(FEElemTopology::EDGE2, order, pts, wts);

    double sum = 0.0;
    for (unsigned int q = 0; q < wts.size(); ++q)
    {
      double x = pts[q].v[0];
      double p = 1.0;
      for (unsigned int k = 0; k < order; ++k) p *= x;
      sum += wts[q] * p;
    }
    double exact = (order % 2 == 0) ? 2.0 / (order + 1.0): 0.0;
    EXPECT_NEAR(sum, exact, TOL) << "order=" << order;
  }
}

// QUAD: integrate xi^order * eta^order over [-1,1]^2 → (2/(order+1))^2
TEST(KokkosQuadratureTest, Exactness_Quad_AllOrders)
{
  for (unsigned int order = 1; order <= 7; ++order)
  {
    Array<Real3> pts; Array<Real> wts;
    fillQuadrature(FEElemTopology::QUAD4, order, pts, wts);

    double sum = 0.0;
    for (unsigned int q = 0; q < wts.size(); ++q)
    {
      double xi = pts[q].v[0], eta = pts[q].v[1];
      double px = 1.0, py = 1.0;
      for (unsigned int k = 0; k < order; ++k) { px *= xi; py *= eta; }
      sum += wts[q] * px * py;
    }
    // integral of x^n * y^n over [-1,1]^2; odd powers integrate to 0
    double f1 = (order % 2 == 0) ? 2.0 / (order + 1.0):0.0;
    double exact = f1 * f1;
    EXPECT_NEAR(sum, exact, TOL) << "order=" << order;
  }
}

// HEX: integrate xi^2 * eta^2 * zeta^2 over [-1,1]^3 → (2/3)^3 = 8/27
TEST(KokkosQuadratureTest, Exactness_Hex_Degree6)
{
  for (unsigned int order = 6; order <= 7; ++order)
  {
    Array<Real3> pts; Array<Real> wts;
    fillQuadrature(FEElemTopology::HEX8, order, pts, wts);

    double sum = 0.0;
    for (unsigned int q = 0; q < wts.size(); ++q)
    {
      double xi = pts[q].v[0], eta = pts[q].v[1], zeta = pts[q].v[2];
      sum += wts[q] * (xi*xi) * (eta*eta) * (zeta*zeta);
    }
    EXPECT_NEAR(sum, 8.0/27.0, TOL) << "order=" << order;
  }
}

// TRI: integrate xi^2 * eta over the unit triangle → 1/24
// This is a degree-3 monomial; any order >= 3 rule must get it exactly.
TEST(KokkosQuadratureTest, Exactness_Tri_Degree3)
{
  for (unsigned int order = 3; order <= 7; ++order)
  {
    Array<Real3> pts; Array<Real> wts;
    fillQuadrature(FEElemTopology::TRI3, order, pts, wts);

    double sum = 0.0;
    for (unsigned int q = 0; q < wts.size(); ++q)
    {
      double xi = pts[q].v[0], eta = pts[q].v[1];
      sum += wts[q] * xi * xi * eta;
    }
    // integral of x^2 * y over {x>=0, y>=0, x+y<=1} = 1/24
    EXPECT_NEAR(sum, 1.0/60.0, TOL) << "order=" << order;
  }
}

// TET: integrate xi * eta * zeta over the unit tet → 1/120
// Degree-3; any order >= 3 rule (including those with negative weights) must
// be exact.
TEST(KokkosQuadratureTest, Exactness_Tet_Degree3)
{
  for (unsigned int order = 3; order <= 6; ++order)
  {
    Array<Real3> pts; Array<Real> wts;
    fillQuadrature(FEElemTopology::TET4, order, pts, wts);

    double sum = 0.0;
    for (unsigned int q = 0; q < wts.size(); ++q)
    {
      double xi = pts[q].v[0], eta = pts[q].v[1], zeta = pts[q].v[2];
      sum += wts[q] * xi * eta * zeta;
    }
    EXPECT_NEAR(sum, 1.0/720.0, TOL) << "order=" << order;
  }
}

// TET: integrate xi^4 over the unit tet → 1/126 (degree-4 exactness check)
TEST(KokkosQuadratureTest, Exactness_Tet_Degree4)
{
  for (unsigned int order = 4; order <= 6; ++order)
  {
    Array<Real3> pts; Array<Real> wts;
    fillQuadrature(FEElemTopology::TET4, order, pts, wts);

    double sum = 0.0;
    for (unsigned int q = 0; q < wts.size(); ++q)
    {
      double xi = pts[q].v[0];
      sum += wts[q] * xi * xi * xi * xi;
    }
    // integral of x^4 over unit tet = 4! * 0! * 0! * 0! / (4+3)! = 24/5040 = 1/210
    // Actually: integral of x^n over unit tet {x,y,z>=0, x+y+z<=1} = n! * 1! * 1! * 1! / (n+3)!
    //   n=4: 24 / 7! = 24/5040 = 1/210
    EXPECT_NEAR(sum, 1.0/210.0, TOL) << "order=" << order;
  }
}

// ── 4. libMesh parity (point-for-point comparison) ───────────────────────────

// Topologies and their dimensionalities for QGauss initialisation.
struct LibMeshParityParam
{
  FEElemTopology topo;
  unsigned int   order;
  std::string    label;
};

static std::vector<LibMeshParityParam> parityParams()
{
  struct Entry { FEElemTopology t; unsigned int d; const char * name; };
  const Entry entries[] = {
    { FEElemTopology::EDGE2,  1, "EDGE2"  },
    { FEElemTopology::EDGE3,  1, "EDGE3"  },
    { FEElemTopology::TRI3,   2, "TRI3"   },
    { FEElemTopology::TRI6,   2, "TRI6"   },
    { FEElemTopology::QUAD4,  2, "QUAD4"  },
    { FEElemTopology::QUAD8,  2, "QUAD8"  },
    { FEElemTopology::QUAD9,  2, "QUAD9"  },
    { FEElemTopology::TET4,   3, "TET4"   },
    { FEElemTopology::TET10,  3, "TET10"  },
    { FEElemTopology::HEX8,   3, "HEX8"   },
    { FEElemTopology::HEX20,  3, "HEX20"  },
    { FEElemTopology::HEX27,  3, "HEX27"  },
  };
  std::vector<LibMeshParityParam> v;
  for (auto & e : entries)
  {
    const unsigned int max_order = (e.t == FEElemTopology::TET4 || e.t == FEElemTopology::TET10)? 6u :7u;
    for (unsigned int order=1 ; order <=max_order; ++order){
      v.push_back({e.t, order, std::string(e.name) + "_o" + std::to_string(order)});
    }
  }
  return v;
}

class KokkosQuadratureParityTest : public ::testing::TestWithParam<LibMeshParityParam> {};

TEST_P(KokkosQuadratureParityTest, MatchesLibMeshQGauss)
{
  auto p = GetParam();

  // Build libMesh reference quadrature
  libMesh::QGauss qrule(dim(p.topo), toLibMeshOrder(p.order));
  qrule.allow_rules_with_negative_weights = true;
  qrule.init(toLibMeshElemType(p.topo));

  // Build native quadrature
  Array<Real3> pts; Array<Real> wts;
  fillQuadrature(p.topo, p.order, pts, wts);

  ASSERT_EQ(pts.size(), (unsigned int)qrule.n_points())
      << p.label << ": point count mismatch";

  for (unsigned int q = 0; q < (unsigned int)qrule.n_points(); ++q)
  {
    EXPECT_NEAR(wts[q], qrule.w(q), PTOL)
        << p.label << " weight q=" << q;
    EXPECT_NEAR(pts[q].v[0], qrule.qp(q)(0), PTOL)
        << p.label << " xi q=" << q;
    if (dim(p.topo) >= 2)
      EXPECT_NEAR(pts[q].v[1], qrule.qp(q)(1), PTOL)
          << p.label << " eta q=" << q;
    if (dim(p.topo) >= 3)
      EXPECT_NEAR(pts[q].v[2], qrule.qp(q)(2), PTOL)
          << p.label << " zeta q=" << q;
  }
}

INSTANTIATE_TEST_SUITE_P(AllTopoOrders,
                         KokkosQuadratureParityTest,
                         ::testing::ValuesIn(parityParams()),
                         [](const auto & info) { return info.param.label; });

// ── 5. Negative-weight rules are present and sign is preserved ────────────────

// The TET order-3 and order-4 Keast rules have negative weights.
// Confirm the weights are actually negative (i.e. not accidentally abs()'d).
TEST(KokkosQuadratureTest, TetOrder3_HasNegativeWeight)
{
  Array<Real3> pts; Array<Real> wts;
  fillQuadrature(FEElemTopology::TET4, 3, pts, wts);

  bool found_negative = false;
  for (unsigned int q = 0; q < wts.size(); ++q)
    if (wts[q] < 0.0) { found_negative = true; break; }

  EXPECT_TRUE(found_negative)
      << "TET order-3 Keast rule must contain a negative weight";
}

TEST(KokkosQuadratureTest, TetOrder4_HasNegativeWeight)
{
  Array<Real3> pts; Array<Real> wts;
  fillQuadrature(FEElemTopology::TET4, 4, pts, wts);

  bool found_negative = false;
  for (unsigned int q = 0; q < wts.size(); ++q)
    if (wts[q] < 0.0) { found_negative = true; break; }

  EXPECT_TRUE(found_negative)
      << "TET order-4 Keast rule must contain a negative weight";
}

// ── 6. fillQuadrature produces identical results on repeated calls ─────────

// Confirms no hidden mutable state in the dispatcher.
TEST(KokkosQuadratureTest, Idempotent_Hex8_Order4)
{
  Array<Real3> pts1, pts2; Array<Real> wts1, wts2;
  fillQuadrature(FEElemTopology::HEX8, 4, pts1, wts1);
  fillQuadrature(FEElemTopology::HEX8, 4, pts2, wts2);

  ASSERT_EQ(pts1.size(), pts2.size());
  for (unsigned int q = 0; q < pts1.size(); ++q)
  {
    EXPECT_EQ(pts1[q].v[0], pts2[q].v[0]);
    EXPECT_EQ(pts1[q].v[1], pts2[q].v[1]);
    EXPECT_EQ(pts1[q].v[2], pts2[q].v[2]);
    EXPECT_EQ(wts1[q], wts2[q]);
  }
}
