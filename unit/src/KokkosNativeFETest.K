//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Tests for Phases 5 & 6: native FE evaluator correctness.
//
// Two things are verified:
//
//   A. FEEvaluator vs libMesh FEBase  (Phase 5 cross-validation)
//      For every (element type, quadrature order) pair, evaluate
//      FEEvaluator<LagrangeTag, ElemTag>::shape/grad_shape at the native
//      quadrature points and compare point-for-point against
//      libMesh FEBase::reinit() output.
//
//   B. nativeShape / nativeGradShape dispatch parity  (Phase 6)
//      The runtime-dispatch functions in KokkosFEReinit.h must produce
//      bit-identical results to direct FEEvaluator calls because they
//      evaluate the same expression at the same coordinates.
//
//   C. mapFaceQpToParent correctness
//      Face quadrature points mapped to parent coordinates must reproduce the
//      same phi values as a libMesh face reinit with the same points.
//
// Run:
//   cd unit && make && ./moose-unit-opt --gtest_filter="KokkosNativeFETest*"

#include "gtest_include.h"

#include "KokkosFELagrange1D.h"
#include "KokkosFELagrange2D.h"
#include "KokkosFELagrange3D.h"
#include "KokkosFEReinit.h"
#include "KokkosQuadrature.h"
#include "KokkosFETypes.h"
#include "KokkosTypes.h"

#include "libmesh/fe_base.h"
#include "libmesh/fe_type.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/reference_elem.h"
#include "libmesh/enum_elem_type.h"
#include "libmesh/enum_fe_family.h"
#include "libmesh/enum_order.h"

#include <cmath>
#include <memory>
#include <string>
#include <vector>

using namespace Moose::Kokkos;

static constexpr double TOL = 1.0e-13;

// ── helpers ───────────────────────────────────────────────────────────────────

struct ElemEntry
{
  FEElemTopology topo;
  libMesh::ElemType lm_type;
  libMesh::Order    lm_order;   // default Lagrange order for this elem type
  unsigned int      dim;
  unsigned int      n_dofs;
  const char *      name;
};

static const ElemEntry ELEMS[] = {
  { FEElemTopology::EDGE2,  libMesh::EDGE2,  libMesh::FIRST,  1,  2, "EDGE2"  },
  { FEElemTopology::EDGE3,  libMesh::EDGE3,  libMesh::SECOND, 1,  3, "EDGE3"  },
  { FEElemTopology::TRI3,   libMesh::TRI3,   libMesh::FIRST,  2,  3, "TRI3"   },
  { FEElemTopology::TRI6,   libMesh::TRI6,   libMesh::SECOND, 2,  6, "TRI6"   },
  { FEElemTopology::QUAD4,  libMesh::QUAD4,  libMesh::FIRST,  2,  4, "QUAD4"  },
  { FEElemTopology::QUAD8,  libMesh::QUAD8,  libMesh::SECOND, 2,  8, "QUAD8"  },
  { FEElemTopology::QUAD9,  libMesh::QUAD9,  libMesh::SECOND, 2,  9, "QUAD9"  },
  { FEElemTopology::TET4,   libMesh::TET4,   libMesh::FIRST,  3,  4, "TET4"   },
  { FEElemTopology::TET10,  libMesh::TET10,  libMesh::SECOND, 3, 10, "TET10"  },
  { FEElemTopology::HEX8,   libMesh::HEX8,   libMesh::FIRST,  3,  8, "HEX8"   },
  { FEElemTopology::HEX20,  libMesh::HEX20,  libMesh::SECOND, 3, 20, "HEX20"  },
  { FEElemTopology::HEX27,  libMesh::HEX27,  libMesh::SECOND, 3, 27, "HEX27"  },
};
static constexpr unsigned int N_ELEMS = sizeof(ELEMS) / sizeof(ELEMS[0]);

// Evaluate FEEvaluator::shape/grad_shape for all (elem, topo, i, qp) via the
// topology switch, writing into pre-allocated raw arrays.
//   phi[i * nqp + qp]
//   grad_phi[i * nqp + qp] component c → grad_phi[(i * nqp + qp) * 3 + c]
static void evalAllNative(FEElemTopology topo,
                           unsigned int n,
                           const Array<Real3> & qpts,
                           std::vector<double> & phi,
                           std::vector<double> & gphi)
{
  unsigned int nqp = qpts.size();
  phi.assign(n * nqp, 0.0);
  gphi.assign(n * nqp * 3, 0.0);

  for (unsigned int i = 0; i < n; ++i)
    for (unsigned int q = 0; q < nqp; ++q)
    {
      Real xi   = qpts[q].v[0];
      Real eta  = qpts[q].v[1];
      Real zeta = qpts[q].v[2];

      phi[i * nqp + q] = nativeShape(topo, i, xi, eta, zeta);

      Real3 g = nativeGradShape(topo, i, xi, eta, zeta);
      gphi[(i * nqp + q) * 3 + 0] = g.v[0];
      gphi[(i * nqp + q) * 3 + 1] = g.v[1];
      gphi[(i * nqp + q) * 3 + 2] = g.v[2];
    }
}

// ── A. FEEvaluator vs libMesh FEBase ─────────────────────────────────────────

struct CrossValParam
{
  unsigned int elem_idx;
  unsigned int order;
  std::string  label;
};

static std::vector<CrossValParam> crossValParams()
{
  std::vector<CrossValParam> v;
  for (unsigned int e = 0; e < N_ELEMS; ++e)
    for (unsigned int order = 1; order <= 5; ++order)
      v.push_back({e, order,
                   std::string(ELEMS[e].name) + "_o" + std::to_string(order)});
  return v;
}

class KokkosNativeFECrossValTest : public ::testing::TestWithParam<CrossValParam> {};

TEST_P(KokkosNativeFECrossValTest, PhiMatchesLibMesh)
{
  auto p = GetParam();
  const auto & e = ELEMS[p.elem_idx];

  // Build native quadrature at this order
  Array<Real3> q_pts; Array<Real> wts;
  fillQuadrature(e.topo, p.order, q_pts, wts);
  const unsigned int nqp = q_pts.size();

  // Build libMesh FE object and attach a QGauss rule at the same order
  libMesh::QGauss qrule(e.dim, static_cast<libMesh::Order>(p.order));
  qrule.allow_rules_with_negative_weights = true;
  auto fe = libMesh::FEBase::build(e.dim, libMesh::FEType(e.lm_order, libMesh::LAGRANGE));
  fe->attach_quadrature_rule(&qrule);


  const auto & phi_lm  = fe->get_phi();
  const auto & dphi_lm = fe->get_dphi();

  const libMesh::Elem * ref = &libMesh::ReferenceElem::get(e.lm_type);
  fe->reinit(ref);

  ASSERT_EQ(phi_lm.size(), (unsigned int)e.n_dofs) << p.label;
  ASSERT_EQ((unsigned int)qrule.n_points(), nqp)   << p.label << ": qp count mismatch vs libMesh";

  for (unsigned int i = 0; i < e.n_dofs; ++i)
    for (unsigned int q = 0; q < nqp; ++q)
    {
      Real xi   = q_pts[q].v[0];
      Real eta  = q_pts[q].v[1];
      Real zeta = q_pts[q].v[2];

      double native_phi = nativeShape(e.topo, i, xi, eta, zeta);
      EXPECT_NEAR(native_phi, phi_lm[i][q], TOL)
          << p.label << " phi i=" << i << " q=" << q;

      Real3 ng = nativeGradShape(e.topo, i, xi, eta, zeta);
      EXPECT_NEAR(ng.v[0], dphi_lm[i][q](0), TOL)
          << p.label << " dphi/dxi i=" << i << " q=" << q;
      if (e.dim >= 2)
        EXPECT_NEAR(ng.v[1], dphi_lm[i][q](1), TOL)
            << p.label << " dphi/deta i=" << i << " q=" << q;
      if (e.dim >= 3)
        EXPECT_NEAR(ng.v[2], dphi_lm[i][q](2), TOL)
            << p.label << " dphi/dzeta i=" << i << " q=" << q;
    }
}

INSTANTIATE_TEST_SUITE_P(AllElemsOrders,
                         KokkosNativeFECrossValTest,
                         ::testing::ValuesIn(crossValParams()),
                         [](const auto & info) { return info.param.label; });

// ── B. nativeShape dispatch is bit-identical to direct FEEvaluator calls ─────
//
// Since both paths evaluate the same inline function at the same coordinates,
// results must be bit-identical (not just within tolerance).

TEST(KokkosNativeFETest, NativeShapeDispatch_BitIdentical_Quad4)
{
  Array<Real3> q_pts; Array<Real> wts;
  fillQuadrature(FEElemTopology::QUAD4, 4, q_pts, wts);

  for (unsigned int i = 0; i < 4; ++i)
    for (unsigned int q = 0; q < q_pts.size(); ++q)
    {
      Real xi = q_pts[q].v[0], eta = q_pts[q].v[1];
      Real direct   = FEEvaluator<LagrangeTag, Quad4Tag>::shape(i, xi, eta, 0.0);
      Real dispatch = nativeShape(FEElemTopology::QUAD4, i, xi, eta, 0.0);
      EXPECT_EQ(direct, dispatch) << "i=" << i << " q=" << q;
    }
}

TEST(KokkosNativeFETest, NativeShapeDispatch_BitIdentical_Tet4)
{
  Array<Real3> q_pts; Array<Real> wts;
  fillQuadrature(FEElemTopology::TET4, 5, q_pts, wts);

  for (unsigned int i = 0; i < 4; ++i)
    for (unsigned int q = 0; q < q_pts.size(); ++q)
    {
      Real xi = q_pts[q].v[0], eta = q_pts[q].v[1], zeta = q_pts[q].v[2];
      Real direct   = FEEvaluator<LagrangeTag, Tet4Tag>::shape(i, xi, eta, zeta);
      Real dispatch = nativeShape(FEElemTopology::TET4, i, xi, eta, zeta);
      EXPECT_EQ(direct, dispatch) << "i=" << i << " q=" << q;
    }
}

TEST(KokkosNativeFETest, NativeShapeDispatch_BitIdentical_Hex27)
{
  Array<Real3> q_pts; Array<Real> wts;
  fillQuadrature(FEElemTopology::HEX27, 3, q_pts, wts);

  for (unsigned int i = 0; i < 27; ++i)
    for (unsigned int q = 0; q < q_pts.size(); ++q)
    {
      Real xi = q_pts[q].v[0], eta = q_pts[q].v[1], zeta = q_pts[q].v[2];
      Real direct   = FEEvaluator<LagrangeTag, Hex27Tag>::shape(i, xi, eta, zeta);
      Real dispatch = nativeShape(FEElemTopology::HEX27, i, xi, eta, zeta);
      EXPECT_EQ(direct, dispatch) << "i=" << i << " q=" << q;
    }
}

TEST(KokkosNativeFETest, NativeGradShapeDispatch_BitIdentical_Tri6)
{
  Array<Real3> q_pts; Array<Real> wts;
  fillQuadrature(FEElemTopology::TRI6, 4, q_pts, wts);

  for (unsigned int i = 0; i < 6; ++i)
    for (unsigned int q = 0; q < q_pts.size(); ++q)
    {
      Real xi = q_pts[q].v[0], eta = q_pts[q].v[1];
      Real3 direct   = FEEvaluator<LagrangeTag, Tri6Tag>::grad_shape(i, xi, eta, 0.0);
      Real3 dispatch = nativeGradShape(FEElemTopology::TRI6, i, xi, eta, 0.0);
      EXPECT_EQ(direct.v[0], dispatch.v[0]) << "v[0] i=" << i << " q=" << q;
      EXPECT_EQ(direct.v[1], dispatch.v[1]) << "v[1] i=" << i << " q=" << q;
    }
}

// ── C. Face qp → parent coordinate mapping ───────────────────────────────────
//
// For a given face side, the parent-reference coordinates produced by
// mapFaceQpToParent must give the same phi values as libMesh's face reinit.
// We test this indirectly: evaluate phi at parent coords via nativeShape and
// compare against the libMesh phi from face reinit.
//
// The test covers QUAD4 (2D), HEX8 (3D), and TRI3 (simplex face).

static void checkFaceMapping(FEElemTopology parent_topo,
                              libMesh::ElemType lm_type,
                              libMesh::Order lm_order,
                              unsigned int parent_dim,
                              unsigned int n_dofs,
                              unsigned int quad_order,
                              const std::string & label)
{
  const libMesh::Elem * ref = &libMesh::ReferenceElem::get(lm_type);
  const auto side_topo = getSideTopology(parent_topo);

  libMesh::QGauss qrule_face(parent_dim - 1,
                              static_cast<libMesh::Order>(quad_order));
  qrule_face.allow_rules_with_negative_weights = true;

  auto fe_face = libMesh::FEBase::build(parent_dim,
                                        libMesh::FEType(lm_order, libMesh::LAGRANGE));
  fe_face->attach_quadrature_rule(&qrule_face);

  const auto & phi_lm = fe_face->get_phi();
  for (unsigned int side = 0; side < ref->n_sides(); ++side)
  {
    // libMesh face reinit — phi stored as phi[dof][qp]
    fe_face->reinit(ref, side);
    unsigned int nqp = qrule_face.n_points();

    // Native face qps (side-element reference coords)
    Array<Real3> face_qpts; Array<Real> face_wts;
    fillQuadrature(side_topo, quad_order, face_qpts, face_wts);
    ASSERT_EQ(face_qpts.size(), nqp)
        << label << " side=" << side << " qp count mismatch";

    // Map face qps to parent reference coords using isoparametric mapping
    auto side_elem = ref->side_ptr(side);
    for (unsigned int q = 0; q < nqp; ++q)
    {
      // Reproduce mapFaceQpToParent logic inline
      // (the actual helper lives inside KokkosAssembly.K as a static function
      //  and is not exposed in a testable header; we replicate it here)
      const Real3 face_pt = face_qpts[q];
      Real3 parent_pt;
      for (unsigned int k = 0; k < side_elem->n_nodes(); ++k)
      {
        Real psi = nativeShape(side_topo, k,
                               face_pt.v[0], face_pt.v[1], face_pt.v[2]);
        const auto & node = side_elem->point(k);
        parent_pt.v[0] += psi * node(0);
        parent_pt.v[1] += psi * node(1);
        parent_pt.v[2] += psi * node(2);
      }

      // Now evaluate parent shape functions at parent_pt and compare to libMesh
      for (unsigned int i = 0; i < n_dofs; ++i)
      {
        double native_phi = nativeShape(parent_topo, i,
                                        parent_pt.v[0], parent_pt.v[1], parent_pt.v[2]);
        EXPECT_NEAR(native_phi, phi_lm[i][q], TOL)
            << label << " side=" << side << " i=" << i << " q=" << q;
      }
    }
  }
}

TEST(KokkosNativeFETest, FaceMapping_Quad4)
{
  checkFaceMapping(FEElemTopology::QUAD4, libMesh::QUAD4,
                   libMesh::FIRST, 2, 4, 3, "QUAD4");
}

TEST(KokkosNativeFETest, FaceMapping_Tri3)
{
  checkFaceMapping(FEElemTopology::TRI3, libMesh::TRI3,
                   libMesh::FIRST, 2, 3, 3, "TRI3");
}

TEST(KokkosNativeFETest, FaceMapping_Hex8)
{
  checkFaceMapping(FEElemTopology::HEX8, libMesh::HEX8,
                   libMesh::FIRST, 3, 8, 3, "HEX8");
}

TEST(KokkosNativeFETest, FaceMapping_Tet4)
{
  checkFaceMapping(FEElemTopology::TET4, libMesh::TET4,
                   libMesh::FIRST, 3, 4, 3, "TET4");
}

TEST(KokkosNativeFETest, FaceMapping_Hex8_HighOrder)
{
  checkFaceMapping(FEElemTopology::HEX8, libMesh::HEX8,
                   libMesh::FIRST, 3, 8, 5, "HEX8_o5");
}

// ── D. nDofs matches libMesh DOF count ───────────────────────────────────────

TEST(KokkosNativeFETest, NDofs_MatchesLibMesh)
{
  for (unsigned int e = 0; e < N_ELEMS; ++e)
  {
    const auto & entry = ELEMS[e];
    unsigned int native_n = nDofs(Moose::Kokkos::FEFamily::LAGRANGE, entry.topo);
    EXPECT_EQ(native_n, entry.n_dofs) << entry.name;
  }
}

// ── E. All FEElemTopology values handled by nativeShape (no default branch) ──
//
// Passing each topology with i=0 at the centroid should return a finite,
// non-NaN value. This guards against accidentally missing a switch case.

TEST(KokkosNativeFETest, NativeShape_NoUnhandledTopology)
{
  // Centroids in reference element coordinates
  const Real3 centroids[] = {
    Real3( 0.0,  0.0, 0.0), // EDGE2
    Real3( 0.0,  0.0, 0.0), // EDGE3
    Real3( 1.0/3.0, 1.0/3.0, 0.0), // TRI3
    Real3( 1.0/3.0, 1.0/3.0, 0.0), // TRI6
    Real3( 0.0,  0.0, 0.0), // QUAD4
    Real3( 0.0,  0.0, 0.0), // QUAD8
    Real3( 0.0,  0.0, 0.0), // QUAD9
    Real3( 1.0/4.0, 1.0/4.0, 1.0/4.0), // TET4
    Real3( 1.0/4.0, 1.0/4.0, 1.0/4.0), // TET10
    Real3( 0.0,  0.0, 0.0), // HEX8
    Real3( 0.0,  0.0, 0.0), // HEX20
    Real3( 0.0,  0.0, 0.0), // HEX27
  };
  static_assert(N_ELEMS == 12, "Update centroids array if element list changes");

  for (unsigned int e = 0; e < N_ELEMS; ++e)
  {
    Real val = nativeShape(ELEMS[e].topo, 0,
                           centroids[e].v[0], centroids[e].v[1], centroids[e].v[2]);
    EXPECT_TRUE(std::isfinite(val)) << ELEMS[e].name << " returned non-finite";
  }
}
