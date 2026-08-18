//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "TransformedDualBasis.h"

#include "libmesh/elem.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/fe.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/enum_order.h"
#include "libmesh/enum_fe_family.h"
#include "libmesh/enum_to_string.h"
#include "libmesh/int_range.h"

#include <memory>
#include <string>
#include <vector>

using namespace libMesh;

namespace
{
// Absolute tolerance for the biorthogonality / diagonal checks. The Gauss rule below integrates the
// (at most cubic) shape products exactly, so the only error is round-off in the dense linear solves
// inside computeTransformedDualCoeffs (~1e-13 for these 6x6/8x8 systems).
const Real tol = 1.0e-9;

// Verify the transformed dual on a single second-order face type: the transformed diagonal dtilde
// is the expected strictly positive value per node, the dual reproduces constants (partition of
// unity), and it is biorthogonal to the transformed basis Ntilde = T N. standard_vertex_diagonal is
// the untransformed vertex value the transform repairs.
void
checkTransformedDual(const ElemType type,
                     const Real vertex_dtilde,
                     const Real mid_dtilde,
                     const Real standard_vertex_diagonal)
{
  SCOPED_TRACE(Utility::enum_to_string(type));

  Parallel::Communicator comm;
  ReplicatedMesh mesh(comm);
  // A single reference face with unit Jacobian so fe->get_JxW() reproduces the raw Gauss weights
  // and the transformed diagonal takes its exact per-node reference value: QUAD8 on [-1,1]^2
  // (identity map); TRI6 on the unit square split into two area-1/2 right triangles, each congruent
  // to the P2 reference triangle (|J| = 1), so per-node integrals hold for either. build_square
  // yields a prepared mesh (unlike a hand-assembled Elem), which fe->reinit requires.
  if (type == TRI6)
    MeshTools::Generation::build_square(mesh, 1, 1, 0., 1., 0., 1., TRI6);
  else
    MeshTools::Generation::build_square(mesh, 1, 1, -1., 1., -1., 1., type);
  auto rng = mesh.active_local_element_ptr_range();
  const Elem * const elem = rng.begin() == rng.end() ? nullptr : *rng.begin();
  ASSERT_NE(elem, nullptr);

  const unsigned int dim = elem->dim();
  const unsigned int n = elem->n_nodes();
  const FEType fe_type(SECOND, LAGRANGE);

  std::unique_ptr<FEBase> fe(FEBase::build(dim, fe_type));
  const std::vector<Real> & JxW = fe->get_JxW();
  const std::vector<std::vector<Real>> & phi = fe->get_phi();
  QGauss qrule(dim, EIGHTH);
  fe->attach_quadrature_rule(&qrule);
  fe->reinit(elem);
  const std::vector<Point> & pts = qrule.get_points();

  DenseMatrix<Real> coeff;
  Moose::Mortar::computeTransformedDualCoeffs(*elem, fe_type, pts, JxW, coeff);
  ASSERT_EQ(coeff.m(), n);
  ASSERT_EQ(coeff.n(), n);

  // Standard dual diagonal d_i = integral(phi_i): non-positive at second-order-face vertices, which
  // is precisely why the standard quadratic dual is ill-posed here.
  std::vector<Real> d(n, 0.0);
  for (const auto i : make_range(n))
    for (const auto q : index_range(JxW))
      d[i] += JxW[q] * phi[i][q];
  EXPECT_NEAR(d[0], standard_vertex_diagonal, tol);

  // The transformed diagonal a node carries: vertices vs. mid-edge nodes. Vertex nodes occupy the
  // local indices [0, n_vertices()); the second-order (mid-edge) nodes follow. (For TRI6/QUAD8
  // n_second_order_adjacent_vertices() returns 2 for every node, so it cannot classify vertices.)
  const unsigned int n_vertices = elem->n_vertices();
  auto expected_dtilde = [&](const unsigned int a)
  { return a < n_vertices ? vertex_dtilde : mid_dtilde; };

  // integral(dual_phi_j) = sum_i coeff(i, j) d_i == dtilde_j, strictly positive (the property the
  // transform restores).
  for (const auto j : make_range(n))
  {
    Real integral = 0.0;
    for (const auto i : make_range(n))
      integral += coeff(i, j) * d[i];
    EXPECT_GT(integral, 0.0);
    EXPECT_NEAR(integral, expected_dtilde(j), tol);
  }

  // The transformed dual reproduces constants: sum_j dual_phi_j(q) == 1 at every point.
  for (const auto q : index_range(JxW))
  {
    Real sum = 0.0;
    for (const auto j : make_range(n))
      for (const auto i : make_range(n))
        sum += coeff(i, j) * phi[i][q];
    EXPECT_NEAR(sum, 1.0, tol);
  }

  // Biorthogonality against the transformed basis Ntilde = T N. Rebuild T here independently of the
  // helper and assert M(k, j) = integral(Ntilde_k dual_phi_j) == delta_kj dtilde_j numerically,
  // rather than trusting a written index convention. alpha = 1/5 must match the helper's value.
  const Real alpha = 1.0 / 5.0;
  DenseMatrix<Real> T(n, n);
  for (const auto a : make_range(n))
    T(a, a) = 1.0;
  for (const auto m : make_range(n_vertices, n))
  {
    T(m, m) = 1.0 - 2.0 * alpha;
    for (const auto v : make_range(elem->n_second_order_adjacent_vertices(m)))
      T(elem->second_order_adjacent_vertex(m, v), m) += alpha;
  }

  DenseMatrix<Real> M(n, n);
  for (const auto q : index_range(JxW))
  {
    std::vector<Real> dual_phi(n, 0.0), ntilde(n, 0.0);
    for (const auto j : make_range(n))
      for (const auto i : make_range(n))
        dual_phi[j] += coeff(i, j) * phi[i][q];
    for (const auto k : make_range(n))
      for (const auto l : make_range(n))
        ntilde[k] += T(k, l) * phi[l][q];
    for (const auto k : make_range(n))
      for (const auto j : make_range(n))
        M(k, j) += JxW[q] * ntilde[k] * dual_phi[j];
  }
  for (const auto k : make_range(n))
    for (const auto j : make_range(n))
      EXPECT_NEAR(M(k, j), k == j ? expected_dtilde(j) : 0.0, tol);
}
}

TEST(TransformedDualBasisTest, supportedTypes)
{
  EXPECT_TRUE(Moose::Mortar::transformedDualBasisSupported(QUAD8));
  EXPECT_TRUE(Moose::Mortar::transformedDualBasisSupported(TRI6));
  EXPECT_FALSE(Moose::Mortar::transformedDualBasisSupported(QUAD9));
  EXPECT_FALSE(Moose::Mortar::transformedDualBasisSupported(QUAD4));
  EXPECT_FALSE(Moose::Mortar::transformedDualBasisSupported(TRI3));
  EXPECT_FALSE(Moose::Mortar::transformedDualBasisSupported(EDGE3));
}

TEST(TransformedDualBasisTest, quad8)
{
  // QUAD8: standard corner diagonal -1/3 -> transformed corner 1/5, mid-edge 4/5.
  checkTransformedDual(QUAD8, 1.0 / 5.0, 4.0 / 5.0, -1.0 / 3.0);
}

TEST(TransformedDualBasisTest, tri6)
{
  // TRI6: standard vertex diagonal 0 -> transformed vertex 1/15, mid-edge 1/10.
  checkTransformedDual(TRI6, 1.0 / 15.0, 1.0 / 10.0, 0.0);
}
