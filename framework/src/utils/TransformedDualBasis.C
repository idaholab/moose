//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TransformedDualBasis.h"

#include "MooseError.h"

#include "libmesh/elem.h"
#include "libmesh/fe_interface.h"
#include "libmesh/dense_vector.h"
#include "libmesh/int_range.h"

using namespace libMesh;

namespace Moose
{
namespace Mortar
{
bool
transformedDualBasisSupported(ElemType elem_type)
{
  return elem_type == TRI6 || elem_type == QUAD8;
}

void
computeTransformedDualCoeffs(const Elem & elem,
                             const FEType & fe_type,
                             const std::vector<Point> & pts,
                             const std::vector<Real> & JxW,
                             DenseMatrix<Real> & dual_coeff)
{
  mooseAssert(transformedDualBasisSupported(elem.type()),
              "The transformed dual basis is only defined for TRI6/QUAD8 faces");
  mooseAssert(pts.size() == JxW.size(), "Point and JxW containers must be the same length");

  const unsigned int dim = elem.dim();
  const unsigned int n = FEInterface::n_shape_functions(fe_type, &elem);
  // This routine requires a nodal (Lagrange) trace basis whose shape index i corresponds to local
  // node i, i.e. n == elem.n_nodes(); the vertex/mid-edge node ordering used to build T below
  // relies on it. Assembly::reinitDual enforces this by falling back to the standard dual whenever
  // the trace basis is not nodal (for example a first-order LM on a quadratic mesh).

  // Standard trace shapes at the integration points: phi[i][qp]. FEInterface::all_shapes fills a
  // pre-sized [n_shapes][n_points] container rather than allocating one, matching libMesh's own
  // FE::reinit_dual_shape_coeffs, so it must be sized before the call.
  std::vector<std::vector<Real>> phi(n, std::vector<Real>(pts.size()));
  FEInterface::all_shapes(dim, fe_type, &elem, pts, phi);

  // Standard mass matrix A(i, j) = integral(phi_i phi_j) and the standard dual diagonal
  // d(i) = integral(phi_i), integrated with the same (pts, JxW) libMesh uses for the untransformed
  // dual so that T = I reproduces it exactly.
  DenseMatrix<Real> A(n, n);
  DenseVector<Real> d(n);
  for (const auto i : make_range(n))
    for (const auto qp : index_range(JxW))
    {
      d(i) += JxW[qp] * phi[i][qp];
      for (const auto j : make_range(n))
        A(i, j) += JxW[qp] * phi[i][qp] * phi[j][qp];
    }

  // Popp (2012) locally-quadratic transform Ntilde = T N: each mid-edge (second-order) node scales
  // its own column by (1 - 2 alpha) and deposits alpha into each of its adjacent vertex rows, while
  // vertex columns stay the identity. This preserves the partition of unity (sum_a Ntilde_a = 1),
  // so the dual also reproduces constants (patch-test consistency).
  // alpha = 1/5 is the single shared scalar Popp, Wohlmuth, Gee & Wall adopt for both TRI6 and
  // QUAD8 in "Dual quadratic mortar finite element methods for 3D finite deformation contact," SIAM
  // J. Sci. Comput. 34(4):B421-B446 (2012), Sec. 4.4.1. They deliberately use one shared alpha
  // rather than the undistorted-element pair alpha = 1/12 (TRI6), 1/5 (QUAD8) of their ref. [20]: a
  // mixed TRI6/QUAD8 mesh needs a single alpha for global displacement continuity, and
  // finite-deformation element distortion makes the ref. [20] piecewise-linear-inclusion criterion
  // unattainable in general. With alpha = 1/5 the transformed diagonal dtilde is strictly positive
  // under full-face integration (QUAD8 -> 1/5, 4/5; TRI6 -> 1/15, 1/10); do not change it without
  // re-deriving that positivity guarantee.
  const Real alpha = 1.0 / 5.0;
  DenseMatrix<Real> T(n, n);
  for (const auto i : make_range(n))
    T(i, i) = 1.0;
  // Vertex nodes occupy the local indices [0, n_vertices()); the second-order (mid-edge) nodes
  // follow at [n_vertices(), n_nodes()). This ordering is the classifier: for TRI6/QUAD8
  // n_second_order_adjacent_vertices() returns 2 for every node (it cannot flag vertices), and
  // second_order_adjacent_vertex(m, .) is only valid for m >= n_vertices(), so the vertex rows
  // are left as the identity and only the trailing mid-edge columns are transformed.
  const unsigned int n_vertices = elem.n_vertices();
  for (const auto m : make_range(n_vertices, n))
  {
    T(m, m) = 1.0 - 2.0 * alpha;
    for (const auto v : make_range(elem.n_second_order_adjacent_vertices(m)))
      T(elem.second_order_adjacent_vertex(m, v), m) += alpha;
  }

  // dtilde = T d. With alpha = 1/5 this is strictly positive for TRI6/QUAD8 when the trace shapes
  // are integrated over the full face; partial contact and correct_edge_dropping integrate only the
  // covered sub-segments, where the positivity guarantee does not hold.
  DenseVector<Real> dtilde(n);
  T.vector_mult(dtilde, d);

  // dual_coeff = A^-1 T^-1 diag(dtilde), solved one column at a time (mirroring libMesh's
  // untransformed dual path; A is SPD). Column j of T^-1 diag(dtilde) is dtilde(j) * (T^-1 e_j).
  // The vector_mult above must precede the lu_solve, which factorizes T in place; T and A are each
  // factorized on the first solve and reused for the remaining columns.
  dual_coeff.resize(n, n);
  DenseVector<Real> unit(n), tinv_col(n), coeffcol(n);
  for (const auto j : make_range(n))
  {
    unit.zero();
    unit(j) = 1.0;
    T.lu_solve(unit, tinv_col);
    tinv_col *= dtilde(j);
    A.cholesky_solve(tinv_col, coeffcol);
    for (const auto row : make_range(n))
      dual_coeff(row, j) = coeffcol(row);
  }
}
}
}
