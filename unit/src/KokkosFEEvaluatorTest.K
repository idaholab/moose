//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Tests for Phase 2: Lagrange FEEvaluator shape function specializations.
//
// Strategy: evaluate shape(i, xi, eta, zeta) and grad_shape(i, ...) at
// a few known reference points and compare against closed-form expected
// values. All tests are CPU-only (MOOSE_KOKKOS_SCOPE is defined for .K
// files compiled with the device compiler, so KOKKOS_INLINE_FUNCTION
// expands to a host+device annotation and the functions are callable here).
//
// Run:
//   cd unit && make && ./moose-unit-opt --gtest_filter="KokkosFEEvaluatorTest*"

#include "gtest_include.h"

#include "libmesh/kokkos/fe_lagrange_1d.h"
#include "libmesh/kokkos/fe_lagrange_2d.h"
#include "libmesh/kokkos/fe_lagrange_3d.h"
#include "libmesh/kokkos/fe_monomial.h"

using namespace libMesh::Kokkos;

// Tolerance for floating-point comparisons
static constexpr double TOL = 1.0e-14;

// ── EDGE2 ─────────────────────────────────────────────────────────────────────

TEST(KokkosFEEvaluatorTest, Edge2_NdofsAndPartitionOfUnity)
{
  using FE = FEEvaluator<LagrangeTag, Edge2Tag>;
  EXPECT_EQ(FE::n_dofs(), 2u);

  // Partition of unity: sum of shape functions = 1 at any xi
  for (double xi : {-1.0, -0.5, 0.0, 0.5, 1.0})
  {
    double sum = 0.0;
    for (unsigned int i = 0; i < FE::n_dofs(); ++i)
      sum += FE::shape(i, xi, 0.0, 0.0);
    EXPECT_NEAR(sum, 1.0, TOL) << "xi=" << xi;
  }
}

TEST(KokkosFEEvaluatorTest, Edge2_NodalValues)
{
  using FE = FEEvaluator<LagrangeTag, Edge2Tag>;
  // phi_i(node_j) = delta_ij
  EXPECT_NEAR(FE::shape(0, -1.0, 0.0, 0.0), 1.0, TOL); // node 0 at xi=-1
  EXPECT_NEAR(FE::shape(0,  1.0, 0.0, 0.0), 0.0, TOL);
  EXPECT_NEAR(FE::shape(1, -1.0, 0.0, 0.0), 0.0, TOL);
  EXPECT_NEAR(FE::shape(1,  1.0, 0.0, 0.0), 1.0, TOL); // node 1 at xi=+1
}

TEST(KokkosFEEvaluatorTest, Edge2_GradConst)
{
  using FE = FEEvaluator<LagrangeTag, Edge2Tag>;
  // Gradients are constant for linear elements
  auto g0 = FE::grad_shape(0, 0.3, 0.0, 0.0);
  EXPECT_NEAR(g0.v[0], -0.5, TOL);
  EXPECT_NEAR(g0.v[1],  0.0, TOL);
  auto g1 = FE::grad_shape(1, 0.3, 0.0, 0.0);
  EXPECT_NEAR(g1.v[0],  0.5, TOL);
}

// ── EDGE3 ─────────────────────────────────────────────────────────────────────

TEST(KokkosFEEvaluatorTest, Edge3_NdofsAndPartitionOfUnity)
{
  using FE = FEEvaluator<LagrangeTag, Edge3Tag>;
  EXPECT_EQ(FE::n_dofs(), 3u);

  for (double xi : {-1.0, -0.5, 0.0, 0.5, 1.0})
  {
    double sum = 0.0;
    for (unsigned int i = 0; i < FE::n_dofs(); ++i)
      sum += FE::shape(i, xi, 0.0, 0.0);
    EXPECT_NEAR(sum, 1.0, TOL) << "xi=" << xi;
  }
}

TEST(KokkosFEEvaluatorTest, Edge3_NodalValues)
{
  using FE = FEEvaluator<LagrangeTag, Edge3Tag>;
  // node 0 at xi=-1, node 1 at xi=+1, node 2 at xi=0
  EXPECT_NEAR(FE::shape(0, -1.0, 0.0, 0.0), 1.0, TOL);
  EXPECT_NEAR(FE::shape(0,  1.0, 0.0, 0.0), 0.0, TOL);
  EXPECT_NEAR(FE::shape(0,  0.0, 0.0, 0.0), 0.0, TOL);
  EXPECT_NEAR(FE::shape(1, -1.0, 0.0, 0.0), 0.0, TOL);
  EXPECT_NEAR(FE::shape(1,  1.0, 0.0, 0.0), 1.0, TOL);
  EXPECT_NEAR(FE::shape(1,  0.0, 0.0, 0.0), 0.0, TOL);
  EXPECT_NEAR(FE::shape(2, -1.0, 0.0, 0.0), 0.0, TOL);
  EXPECT_NEAR(FE::shape(2,  1.0, 0.0, 0.0), 0.0, TOL);
  EXPECT_NEAR(FE::shape(2,  0.0, 0.0, 0.0), 1.0, TOL); // midpoint node
}

TEST(KokkosFEEvaluatorTest, Edge3_GradAtMidpoint)
{
  using FE = FEEvaluator<LagrangeTag, Edge3Tag>;
  // At xi=0: dL_0/dxi = -0.5, dL_1/dxi = 0.5, dL_2/dxi = 0
  auto g0 = FE::grad_shape(0, 0.0, 0.0, 0.0);
  EXPECT_NEAR(g0.v[0], -0.5, TOL);
  auto g1 = FE::grad_shape(1, 0.0, 0.0, 0.0);
  EXPECT_NEAR(g1.v[0],  0.5, TOL);
  auto g2 = FE::grad_shape(2, 0.0, 0.0, 0.0);
  EXPECT_NEAR(g2.v[0],  0.0, TOL);
}

// ── TRI3 ──────────────────────────────────────────────────────────────────────

TEST(KokkosFEEvaluatorTest, Tri3_NdofsAndPartitionOfUnity)
{
  using FE = FEEvaluator<LagrangeTag, Tri3Tag>;
  EXPECT_EQ(FE::n_dofs(), 3u);

  // Interior and boundary points of the unit triangle
  for (auto [xi, eta] : std::initializer_list<std::pair<double,double>>{
      {0.0,0.0}, {1.0,0.0}, {0.0,1.0}, {0.25,0.25}, {0.5,0.25}})
  {
    double sum = 0.0;
    for (unsigned int i = 0; i < FE::n_dofs(); ++i)
      sum += FE::shape(i, xi, eta, 0.0);
    EXPECT_NEAR(sum, 1.0, TOL) << "xi=" << xi << " eta=" << eta;
  }
}

TEST(KokkosFEEvaluatorTest, Tri3_NodalValues)
{
  using FE = FEEvaluator<LagrangeTag, Tri3Tag>;
  EXPECT_NEAR(FE::shape(0, 0.0, 0.0, 0.0), 1.0, TOL); // node 0 at (0,0)
  EXPECT_NEAR(FE::shape(1, 0.0, 0.0, 0.0), 0.0, TOL);
  EXPECT_NEAR(FE::shape(2, 0.0, 0.0, 0.0), 0.0, TOL);
  EXPECT_NEAR(FE::shape(1, 1.0, 0.0, 0.0), 1.0, TOL); // node 1 at (1,0)
  EXPECT_NEAR(FE::shape(2, 0.0, 1.0, 0.0), 1.0, TOL); // node 2 at (0,1)
}

TEST(KokkosFEEvaluatorTest, Tri3_GradConst)
{
  using FE = FEEvaluator<LagrangeTag, Tri3Tag>;
  auto g0 = FE::grad_shape(0, 0.25, 0.25, 0.0);
  EXPECT_NEAR(g0.v[0], -1.0, TOL);
  EXPECT_NEAR(g0.v[1], -1.0, TOL);
  auto g1 = FE::grad_shape(1, 0.25, 0.25, 0.0);
  EXPECT_NEAR(g1.v[0],  1.0, TOL);
  EXPECT_NEAR(g1.v[1],  0.0, TOL);
  auto g2 = FE::grad_shape(2, 0.25, 0.25, 0.0);
  EXPECT_NEAR(g2.v[0],  0.0, TOL);
  EXPECT_NEAR(g2.v[1],  1.0, TOL);
}

// ── TRI6 ──────────────────────────────────────────────────────────────────────

TEST(KokkosFEEvaluatorTest, Tri6_NdofsAndPartitionOfUnity)
{
  using FE = FEEvaluator<LagrangeTag, Tri6Tag>;
  EXPECT_EQ(FE::n_dofs(), 6u);

  for (auto [xi, eta] : std::initializer_list<std::pair<double,double>>{
      {0.0,0.0}, {1.0,0.0}, {0.0,1.0}, {0.5,0.0}, {0.5,0.5}, {0.0,0.5}, {0.25,0.25}})
  {
    double sum = 0.0;
    for (unsigned int i = 0; i < FE::n_dofs(); ++i)
      sum += FE::shape(i, xi, eta, 0.0);
    EXPECT_NEAR(sum, 1.0, TOL) << "xi=" << xi << " eta=" << eta;
  }
}

TEST(KokkosFEEvaluatorTest, Tri6_NodalValues)
{
  using FE = FEEvaluator<LagrangeTag, Tri6Tag>;
  // Corner nodes: 0→(0,0), 1→(1,0), 2→(0,1)
  // Mid-edge nodes: 3→(0.5,0), 4→(0.5,0.5), 5→(0,0.5)
  const double nodes[6][2] = {{0,0},{1,0},{0,1},{0.5,0},{0.5,0.5},{0,0.5}};
  for (unsigned int i = 0; i < 6; ++i)
    for (unsigned int j = 0; j < 6; ++j)
    {
      double val = FE::shape(i, nodes[j][0], nodes[j][1], 0.0);
      EXPECT_NEAR(val, (i == j) ? 1.0 : 0.0, TOL)
        << "phi_" << i << " at node " << j;
    }
}

// ── QUAD4 ─────────────────────────────────────────────────────────────────────

TEST(KokkosFEEvaluatorTest, Quad4_NdofsAndPartitionOfUnity)
{
  using FE = FEEvaluator<LagrangeTag, Quad4Tag>;
  EXPECT_EQ(FE::n_dofs(), 4u);

  for (auto [xi, eta] : std::initializer_list<std::pair<double,double>>{
      {-1,-1},{1,-1},{1,1},{-1,1},{0,0},{0.3,-0.7}})
  {
    double sum = 0.0;
    for (unsigned int i = 0; i < FE::n_dofs(); ++i)
      sum += FE::shape(i, xi, eta, 0.0);
    EXPECT_NEAR(sum, 1.0, TOL) << "xi=" << xi << " eta=" << eta;
  }
}

TEST(KokkosFEEvaluatorTest, Quad4_NodalValues)
{
  using FE = FEEvaluator<LagrangeTag, Quad4Tag>;
  const double nodes[4][2] = {{-1,-1},{1,-1},{1,1},{-1,1}};
  for (unsigned int i = 0; i < 4; ++i)
    for (unsigned int j = 0; j < 4; ++j)
    {
      double val = FE::shape(i, nodes[j][0], nodes[j][1], 0.0);
      EXPECT_NEAR(val, (i == j) ? 1.0 : 0.0, TOL)
        << "phi_" << i << " at node " << j;
    }
}

// ── QUAD8 ─────────────────────────────────────────────────────────────────────

TEST(KokkosFEEvaluatorTest, Quad8_NdofsAndPartitionOfUnity)
{
  using FE = FEEvaluator<LagrangeTag, Quad8Tag>;
  EXPECT_EQ(FE::n_dofs(), 8u);

  for (auto [xi, eta] : std::initializer_list<std::pair<double,double>>{
      {-1,-1},{1,-1},{1,1},{-1,1},{0,-1},{1,0},{0,1},{-1,0},{0,0},{0.3,-0.4}})
  {
    double sum = 0.0;
    for (unsigned int i = 0; i < FE::n_dofs(); ++i)
      sum += FE::shape(i, xi, eta, 0.0);
    EXPECT_NEAR(sum, 1.0, TOL) << "xi=" << xi << " eta=" << eta;
  }
}

TEST(KokkosFEEvaluatorTest, Quad8_NodalValues)
{
  using FE = FEEvaluator<LagrangeTag, Quad8Tag>;
  const double nodes[8][2] = {{-1,-1},{1,-1},{1,1},{-1,1},{0,-1},{1,0},{0,1},{-1,0}};
  for (unsigned int i = 0; i < 8; ++i)
    for (unsigned int j = 0; j < 8; ++j)
    {
      double val = FE::shape(i, nodes[j][0], nodes[j][1], 0.0);
      EXPECT_NEAR(val, (i == j) ? 1.0 : 0.0, TOL)
        << "phi_" << i << " at node " << j;
    }
}

// ── QUAD9 ─────────────────────────────────────────────────────────────────────

TEST(KokkosFEEvaluatorTest, Quad9_NdofsAndPartitionOfUnity)
{
  using FE = FEEvaluator<LagrangeTag, Quad9Tag>;
  EXPECT_EQ(FE::n_dofs(), 9u);

  for (auto [xi, eta] : std::initializer_list<std::pair<double,double>>{
      {-1,-1},{1,-1},{1,1},{-1,1},{0,-1},{1,0},{0,1},{-1,0},{0,0},{0.3,-0.7}})
  {
    double sum = 0.0;
    for (unsigned int i = 0; i < FE::n_dofs(); ++i)
      sum += FE::shape(i, xi, eta, 0.0);
    EXPECT_NEAR(sum, 1.0, TOL) << "xi=" << xi << " eta=" << eta;
  }
}

TEST(KokkosFEEvaluatorTest, Quad9_NodalValues)
{
  using FE = FEEvaluator<LagrangeTag, Quad9Tag>;
  // libMesh node ordering: corners(-1±1), then midsides at 0, then center
  const double nodes[9][2] = {
    {-1,-1},{1,-1},{1,1},{-1,1},   // corners
    {0,-1},{1,0},{0,1},{-1,0},     // midsides
    {0,0}                          // center
  };
  for (unsigned int i = 0; i < 9; ++i)
    for (unsigned int j = 0; j < 9; ++j)
    {
      double val = FE::shape(i, nodes[j][0], nodes[j][1], 0.0);
      EXPECT_NEAR(val, (i == j) ? 1.0 : 0.0, TOL)
        << "phi_" << i << " at node " << j;
    }
}

// ── TET4 ──────────────────────────────────────────────────────────────────────

TEST(KokkosFEEvaluatorTest, Tet4_NdofsAndPartitionOfUnity)
{
  using FE = FEEvaluator<LagrangeTag, Tet4Tag>;
  EXPECT_EQ(FE::n_dofs(), 4u);

  for (auto [xi,eta,zeta] : std::initializer_list<std::tuple<double,double,double>>{
      {0,0,0},{1,0,0},{0,1,0},{0,0,1},{0.25,0.25,0.25}})
  {
    double sum = 0.0;
    for (unsigned int i = 0; i < FE::n_dofs(); ++i)
      sum += FE::shape(i, xi, eta, zeta);
    EXPECT_NEAR(sum, 1.0, TOL);
  }
}

TEST(KokkosFEEvaluatorTest, Tet4_NodalValues)
{
  using FE = FEEvaluator<LagrangeTag, Tet4Tag>;
  const double nodes[4][3] = {{0,0,0},{1,0,0},{0,1,0},{0,0,1}};
  for (unsigned int i = 0; i < 4; ++i)
    for (unsigned int j = 0; j < 4; ++j)
    {
      double val = FE::shape(i, nodes[j][0], nodes[j][1], nodes[j][2]);
      EXPECT_NEAR(val, (i == j) ? 1.0 : 0.0, TOL)
        << "phi_" << i << " at node " << j;
    }
}

// ── TET10 ─────────────────────────────────────────────────────────────────────

TEST(KokkosFEEvaluatorTest, Tet10_NdofsAndPartitionOfUnity)
{
  using FE = FEEvaluator<LagrangeTag, Tet10Tag>;
  EXPECT_EQ(FE::n_dofs(), 10u);

  for (auto [xi,eta,zeta] : std::initializer_list<std::tuple<double,double,double>>{
      {0,0,0},{1,0,0},{0,1,0},{0,0,1},
      {0.5,0,0},{0.5,0.5,0},{0,0.5,0},{0,0,0.5},{0.5,0,0.5},{0,0.5,0.5},
      {0.2,0.3,0.1}})
  {
    double sum = 0.0;
    for (unsigned int i = 0; i < FE::n_dofs(); ++i)
      sum += FE::shape(i, xi, eta, zeta);
    EXPECT_NEAR(sum, 1.0, TOL);
  }
}

TEST(KokkosFEEvaluatorTest, Tet10_NodalValues)
{
  using FE = FEEvaluator<LagrangeTag, Tet10Tag>;
  // Corners: 0→(0,0,0), 1→(1,0,0), 2→(0,1,0), 3→(0,0,1)
  // Edge midpoints: 4→(0.5,0,0), 5→(0.5,0.5,0), 6→(0,0.5,0),
  //                 7→(0,0,0.5), 8→(0.5,0,0.5), 9→(0,0.5,0.5)
  const double nodes[10][3] = {
    {0,0,0},{1,0,0},{0,1,0},{0,0,1},
    {0.5,0,0},{0.5,0.5,0},{0,0.5,0},{0,0,0.5},{0.5,0,0.5},{0,0.5,0.5}
  };
  for (unsigned int i = 0; i < 10; ++i)
    for (unsigned int j = 0; j < 10; ++j)
    {
      double val = FE::shape(i, nodes[j][0], nodes[j][1], nodes[j][2]);
      EXPECT_NEAR(val, (i == j) ? 1.0 : 0.0, TOL)
        << "phi_" << i << " at node " << j;
    }
}

// ── HEX8 ──────────────────────────────────────────────────────────────────────

TEST(KokkosFEEvaluatorTest, Hex8_NdofsAndPartitionOfUnity)
{
  using FE = FEEvaluator<LagrangeTag, Hex8Tag>;
  EXPECT_EQ(FE::n_dofs(), 8u);

  for (auto [xi,eta,zeta] : std::initializer_list<std::tuple<double,double,double>>{
      {-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},
      {-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1},{0,0,0},{0.3,-0.4,0.7}})
  {
    double sum = 0.0;
    for (unsigned int i = 0; i < FE::n_dofs(); ++i)
      sum += FE::shape(i, xi, eta, zeta);
    EXPECT_NEAR(sum, 1.0, TOL);
  }
}

TEST(KokkosFEEvaluatorTest, Hex8_NodalValues)
{
  using FE = FEEvaluator<LagrangeTag, Hex8Tag>;
  const double nodes[8][3] = {
    {-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},
    {-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}
  };
  for (unsigned int i = 0; i < 8; ++i)
    for (unsigned int j = 0; j < 8; ++j)
    {
      double val = FE::shape(i, nodes[j][0], nodes[j][1], nodes[j][2]);
      EXPECT_NEAR(val, (i == j) ? 1.0 : 0.0, TOL)
        << "phi_" << i << " at node " << j;
    }
}

// ── HEX20 ─────────────────────────────────────────────────────────────────────

TEST(KokkosFEEvaluatorTest, Hex20_NdofsAndPartitionOfUnity)
{
  using FE = FEEvaluator<LagrangeTag, Hex20Tag>;
  EXPECT_EQ(FE::n_dofs(), 20u);

  for (auto [xi,eta,zeta] : std::initializer_list<std::tuple<double,double,double>>{
      {0,0,0},{0.3,-0.4,0.7},{-0.5,0.5,-0.5}})
  {
    double sum = 0.0;
    for (unsigned int i = 0; i < FE::n_dofs(); ++i)
      sum += FE::shape(i, xi, eta, zeta);
    EXPECT_NEAR(sum, 1.0, TOL);
  }
}

TEST(KokkosFEEvaluatorTest, Hex20_NodalValues)
{
  using FE = FEEvaluator<LagrangeTag, Hex20Tag>;
  const double nodes[20][3] = {
    // corners
    {-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},
    {-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1},
    // xi-edge midsides
    {0,-1,-1},{0,1,-1},{0,-1,1},{0,1,1},
    // eta-edge midsides
    {1,0,-1},{-1,0,-1},{1,0,1},{-1,0,1},
    // zeta-edge midsides
    {-1,-1,0},{1,-1,0},{1,1,0},{-1,1,0}
  };
  // The node table above must match the case ordering in shape().
  // Reorder to match libMesh node indices 0-19:
  //  0-7: corners (same)
  //  8:  xi-edge sy=-1 sz=-1  → (0,-1,-1)
  //  9:  eta-edge sx=+1 sz=-1 → (1, 0,-1)
  // 10:  xi-edge sy=+1 sz=-1  → (0,+1,-1)
  // 11:  eta-edge sx=-1 sz=-1 → (-1,0,-1)
  // 12:  zeta-edge sx=-1 sy=-1 → (-1,-1,0)
  // 13:  zeta-edge sx=+1 sy=-1 → (+1,-1,0)
  // 14:  zeta-edge sx=+1 sy=+1 → (+1,+1,0)
  // 15:  zeta-edge sx=-1 sy=+1 → (-1,+1,0)
  // 16:  xi-edge sy=-1 sz=+1  → (0,-1,+1)
  // 17:  eta-edge sx=+1 sz=+1 → (+1,0,+1)
  // 18:  xi-edge sy=+1 sz=+1  → (0,+1,+1)
  // 19:  eta-edge sx=-1 sz=+1 → (-1,0,+1)
  const double ordered_nodes[20][3] = {
    {-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},
    {-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1},
    {0,-1,-1},{1,0,-1},{0,1,-1},{-1,0,-1},
    {-1,-1,0},{1,-1,0},{1,1,0},{-1,1,0},
    {0,-1,1},{1,0,1},{0,1,1},{-1,0,1}
  };
  for (unsigned int i = 0; i < 20; ++i)
    for (unsigned int j = 0; j < 20; ++j)
    {
      double val = FE::shape(i,
                             ordered_nodes[j][0],
                             ordered_nodes[j][1],
                             ordered_nodes[j][2]);
      EXPECT_NEAR(val, (i == j) ? 1.0 : 0.0, TOL)
        << "phi_" << i << " at node " << j;
    }
}

// ── HEX27 ─────────────────────────────────────────────────────────────────────

TEST(KokkosFEEvaluatorTest, Hex27_NdofsAndPartitionOfUnity)
{
  using FE = FEEvaluator<LagrangeTag, Hex27Tag>;
  EXPECT_EQ(FE::n_dofs(), 27u);

  for (auto [xi,eta,zeta] : std::initializer_list<std::tuple<double,double,double>>{
      {0,0,0},{0.3,-0.4,0.7},{-0.5,0.5,-0.5},{1,1,1},{-1,-1,-1}})
  {
    double sum = 0.0;
    for (unsigned int i = 0; i < FE::n_dofs(); ++i)
      sum += FE::shape(i, xi, eta, zeta);
    EXPECT_NEAR(sum, 1.0, TOL);
  }
}

TEST(KokkosFEEvaluatorTest, Hex27_NodalValues)
{
  using FE = FEEvaluator<LagrangeTag, Hex27Tag>;
  // Node coordinates derived from i0/i1/i2 tables and L_k evaluation:
  // index 0→t=-1, index 1→t=+1, index 2→t=0
  static const int idx_to_coord[] = {-1, 1, 0}; // index 0→-1, 1→+1, 2→0
  static const unsigned int i0[] =
    {0,1,1,0, 0,1,1,0, 2,1,2,0, 0,1,1,0, 2,1,2,0, 2,2,1,2,0,2,2};
  static const unsigned int i1[] =
    {0,0,1,1, 0,0,1,1, 0,2,1,2, 0,0,1,1, 0,2,1,2, 2,0,2,1,2,2,2};
  static const unsigned int i2[] =
    {0,0,0,0, 1,1,1,1, 0,0,0,0, 2,2,2,2, 1,1,1,1, 0,2,2,2,2,1,2};

  for (unsigned int i = 0; i < 27; ++i)
  {
    double xi   = idx_to_coord[i0[i]];
    double eta  = idx_to_coord[i1[i]];
    double zeta = idx_to_coord[i2[i]];
    for (unsigned int j = 0; j < 27; ++j)
    {
      double nxi   = idx_to_coord[i0[j]];
      double neta  = idx_to_coord[i1[j]];
      double nzeta = idx_to_coord[i2[j]];
      double val = FE::shape(i, nxi, neta, nzeta);
      EXPECT_NEAR(val, (i == j) ? 1.0 : 0.0, TOL)
        << "phi_" << i << " at node " << j;
    }
  }
}

// ── Gradient consistency: sum of gradients = 0 (partition of unity) ──────────
// If sum_i phi_i = 1 everywhere, then sum_i grad phi_i = 0.

TEST(KokkosFEEvaluatorTest, GradSumZero_Edge2)
{
  using FE = FEEvaluator<LagrangeTag, Edge2Tag>;
  for (double xi : {-0.7, 0.0, 0.4})
  {
    double gsum = 0.0;
    for (unsigned int i = 0; i < FE::n_dofs(); ++i)
      gsum += FE::grad_shape(i, xi, 0.0, 0.0).v[0];
    EXPECT_NEAR(gsum, 0.0, TOL);
  }
}

TEST(KokkosFEEvaluatorTest, GradSumZero_Quad4)
{
  using FE = FEEvaluator<LagrangeTag, Quad4Tag>;
  for (auto [xi, eta] : std::initializer_list<std::pair<double,double>>{{0.3,-0.2},{-0.6,0.7}})
  {
    double gxi = 0.0, geta = 0.0;
    for (unsigned int i = 0; i < FE::n_dofs(); ++i)
    {
      auto g = FE::grad_shape(i, xi, eta, 0.0);
      gxi  += g.v[0];
      geta += g.v[1];
    }
    EXPECT_NEAR(gxi,  0.0, TOL);
    EXPECT_NEAR(geta, 0.0, TOL);
  }
}

TEST(KokkosFEEvaluatorTest, GradSumZero_Hex8)
{
  using FE = FEEvaluator<LagrangeTag, Hex8Tag>;
  for (auto [xi,eta,zeta] : std::initializer_list<std::tuple<double,double,double>>{
      {0.3,-0.2,0.5},{-0.6,0.7,-0.4}})
  {
    double gxi = 0.0, geta = 0.0, gzeta = 0.0;
    for (unsigned int i = 0; i < FE::n_dofs(); ++i)
    {
      auto g = FE::grad_shape(i, xi, eta, zeta);
      gxi   += g.v[0];
      geta  += g.v[1];
      gzeta += g.v[2];
    }
    EXPECT_NEAR(gxi,   0.0, TOL);
    EXPECT_NEAR(geta,  0.0, TOL);
    EXPECT_NEAR(gzeta, 0.0, TOL);
  }
}

// ── MONOMIAL Dim1 (EDGE elements) ────────────────────────────────────────────
// Basis: {1, xi, xi², xi³, ...}  (NOT a partition of unity)

TEST(KokkosFEEvaluatorTest, MonomialDim1_Order0_Basis)
{
  using FE = FEEvaluator<MonomialTag, Dim1Tag, 0>;
  EXPECT_EQ(FE::n_dofs(), 1u);
  // phi_0 = 1 everywhere; grad = 0
  for (double xi : {-1.0, -0.5, 0.0, 0.5, 1.0})
  {
    EXPECT_NEAR(FE::shape(0, xi, 0.0, 0.0), 1.0, TOL) << "xi=" << xi;
    auto g = FE::grad_shape(0, xi, 0.0, 0.0);
    EXPECT_NEAR(g.v[0], 0.0, TOL);
  }
}

TEST(KokkosFEEvaluatorTest, MonomialDim1_Order1_Basis)
{
  using FE = FEEvaluator<MonomialTag, Dim1Tag, 1>;
  EXPECT_EQ(FE::n_dofs(), 2u);
  // phi_0 = 1, phi_1 = xi
  for (double xi : {-0.7, 0.0, 0.3, 1.0})
  {
    EXPECT_NEAR(FE::shape(0, xi, 0.0, 0.0), 1.0, TOL) << "xi=" << xi;
    EXPECT_NEAR(FE::shape(1, xi, 0.0, 0.0), xi,  TOL) << "xi=" << xi;
    auto g1 = FE::grad_shape(1, xi, 0.0, 0.0);
    EXPECT_NEAR(g1.v[0], 1.0, TOL);
  }
}

TEST(KokkosFEEvaluatorTest, MonomialDim1_Order2_Basis)
{
  using FE = FEEvaluator<MonomialTag, Dim1Tag, 2>;
  EXPECT_EQ(FE::n_dofs(), 3u);
  // phi_0 = 1, phi_1 = xi, phi_2 = xi²
  for (double xi : {-0.5, 0.0, 0.4})
  {
    EXPECT_NEAR(FE::shape(0, xi, 0.0, 0.0), 1.0,    TOL) << "xi=" << xi;
    EXPECT_NEAR(FE::shape(1, xi, 0.0, 0.0), xi,     TOL) << "xi=" << xi;
    EXPECT_NEAR(FE::shape(2, xi, 0.0, 0.0), xi * xi, TOL) << "xi=" << xi;
    auto g2 = FE::grad_shape(2, xi, 0.0, 0.0);
    EXPECT_NEAR(g2.v[0], 2.0 * xi, TOL);
    EXPECT_NEAR(g2.v[1], 0.0,      TOL);
  }
}

// ── MONOMIAL Dim2 (TRI and QUAD share this evaluator) ────────────────────────
// Graded-lex: {1, xi, eta, xi², xi·eta, eta², xi³, ...}

TEST(KokkosFEEvaluatorTest, MonomialDim2_Order0_Basis)
{
  using FE = FEEvaluator<MonomialTag, Dim2Tag, 0>;
  EXPECT_EQ(FE::n_dofs(), 1u);
  for (auto [xi, eta] : std::initializer_list<std::pair<double,double>>{
      {0.0,0.0},{0.3,0.4},{-0.5,0.5}})
  {
    EXPECT_NEAR(FE::shape(0, xi, eta, 0.0), 1.0, TOL) << "xi=" << xi << " eta=" << eta;
    auto g = FE::grad_shape(0, xi, eta, 0.0);
    EXPECT_NEAR(g.v[0], 0.0, TOL);
    EXPECT_NEAR(g.v[1], 0.0, TOL);
  }
}

TEST(KokkosFEEvaluatorTest, MonomialDim2_Order1_Basis)
{
  using FE = FEEvaluator<MonomialTag, Dim2Tag, 1>;
  EXPECT_EQ(FE::n_dofs(), 3u);
  // i=0:1, i=1:xi, i=2:eta
  const double xi = 0.3, eta = -0.4;
  EXPECT_NEAR(FE::shape(0, xi, eta, 0.0), 1.0, TOL);
  EXPECT_NEAR(FE::shape(1, xi, eta, 0.0), xi,  TOL);
  EXPECT_NEAR(FE::shape(2, xi, eta, 0.0), eta, TOL);
  auto g1 = FE::grad_shape(1, xi, eta, 0.0);
  EXPECT_NEAR(g1.v[0], 1.0, TOL); EXPECT_NEAR(g1.v[1], 0.0, TOL);
  auto g2 = FE::grad_shape(2, xi, eta, 0.0);
  EXPECT_NEAR(g2.v[0], 0.0, TOL); EXPECT_NEAR(g2.v[1], 1.0, TOL);
}

TEST(KokkosFEEvaluatorTest, MonomialDim2_Order2_Basis)
{
  using FE = FEEvaluator<MonomialTag, Dim2Tag, 2>;
  EXPECT_EQ(FE::n_dofs(), 6u);
  // i: 0→1, 1→xi, 2→eta, 3→xi², 4→xi·eta, 5→eta²
  const double xi = 0.3, eta = 0.5;
  EXPECT_NEAR(FE::shape(0, xi, eta, 0.0), 1.0,      TOL);
  EXPECT_NEAR(FE::shape(1, xi, eta, 0.0), xi,       TOL);
  EXPECT_NEAR(FE::shape(2, xi, eta, 0.0), eta,      TOL);
  EXPECT_NEAR(FE::shape(3, xi, eta, 0.0), xi * xi,  TOL);
  EXPECT_NEAR(FE::shape(4, xi, eta, 0.0), xi * eta, TOL);
  EXPECT_NEAR(FE::shape(5, xi, eta, 0.0), eta * eta, TOL);
  // grad(xi·eta) = {eta, xi, 0}
  auto g4 = FE::grad_shape(4, xi, eta, 0.0);
  EXPECT_NEAR(g4.v[0], eta, TOL);
  EXPECT_NEAR(g4.v[1], xi,  TOL);
  EXPECT_NEAR(g4.v[2], 0.0, TOL);
  // grad(eta²) = {0, 2·eta, 0}
  auto g5 = FE::grad_shape(5, xi, eta, 0.0);
  EXPECT_NEAR(g5.v[0], 0.0,       TOL);
  EXPECT_NEAR(g5.v[1], 2.0 * eta, TOL);
}

// ── MONOMIAL Dim3 (TET/HEX/PRISM/PYRAMID share this evaluator) ───────────────
// Graded-lex: {1, xi, eta, zeta, xi², xi·eta, xi·zeta, eta², eta·zeta, zeta², ...}

TEST(KokkosFEEvaluatorTest, MonomialDim3_Order0_Basis)
{
  using FE = FEEvaluator<MonomialTag, Dim3Tag, 0>;
  EXPECT_EQ(FE::n_dofs(), 1u);
  EXPECT_NEAR(FE::shape(0, 0.1, 0.2, 0.3), 1.0, TOL);
  auto g = FE::grad_shape(0, 0.1, 0.2, 0.3);
  EXPECT_NEAR(g.v[0], 0.0, TOL);
  EXPECT_NEAR(g.v[1], 0.0, TOL);
  EXPECT_NEAR(g.v[2], 0.0, TOL);
}

TEST(KokkosFEEvaluatorTest, MonomialDim3_Order1_Basis)
{
  using FE = FEEvaluator<MonomialTag, Dim3Tag, 1>;
  EXPECT_EQ(FE::n_dofs(), 4u);
  // i=0:1, i=1:xi, i=2:eta, i=3:zeta
  const double xi = 0.2, eta = 0.3, zeta = 0.1;
  EXPECT_NEAR(FE::shape(0, xi, eta, zeta), 1.0,  TOL);
  EXPECT_NEAR(FE::shape(1, xi, eta, zeta), xi,   TOL);
  EXPECT_NEAR(FE::shape(2, xi, eta, zeta), eta,  TOL);
  EXPECT_NEAR(FE::shape(3, xi, eta, zeta), zeta, TOL);
  auto g3 = FE::grad_shape(3, xi, eta, zeta);
  EXPECT_NEAR(g3.v[0], 0.0, TOL);
  EXPECT_NEAR(g3.v[1], 0.0, TOL);
  EXPECT_NEAR(g3.v[2], 1.0, TOL);
}

TEST(KokkosFEEvaluatorTest, MonomialDim3_Order2_Basis)
{
  using FE = FEEvaluator<MonomialTag, Dim3Tag, 2>;
  EXPECT_EQ(FE::n_dofs(), 10u);
  // i: 0→1, 1→xi, 2→eta, 3→zeta, 4→xi², 5→xi·eta, 6→xi·zeta,
  //    7→eta², 8→eta·zeta, 9→zeta²
  const double xi = 0.2, eta = 0.3, zeta = -0.4;
  EXPECT_NEAR(FE::shape(0, xi, eta, zeta), 1.0,        TOL);
  EXPECT_NEAR(FE::shape(4, xi, eta, zeta), xi * xi,    TOL);
  EXPECT_NEAR(FE::shape(5, xi, eta, zeta), xi * eta,   TOL);
  EXPECT_NEAR(FE::shape(6, xi, eta, zeta), xi * zeta,  TOL);
  EXPECT_NEAR(FE::shape(7, xi, eta, zeta), eta * eta,  TOL);
  EXPECT_NEAR(FE::shape(8, xi, eta, zeta), eta * zeta, TOL);
  EXPECT_NEAR(FE::shape(9, xi, eta, zeta), zeta * zeta, TOL);
  // grad(eta·zeta) = {0, zeta, eta}
  auto g8 = FE::grad_shape(8, xi, eta, zeta);
  EXPECT_NEAR(g8.v[0], 0.0,  TOL);
  EXPECT_NEAR(g8.v[1], zeta, TOL);
  EXPECT_NEAR(g8.v[2], eta,  TOL);
  // grad(xi·zeta) = {zeta, 0, xi}
  auto g6 = FE::grad_shape(6, xi, eta, zeta);
  EXPECT_NEAR(g6.v[0], zeta, TOL);
  EXPECT_NEAR(g6.v[1], 0.0,  TOL);
  EXPECT_NEAR(g6.v[2], xi,   TOL);
}
