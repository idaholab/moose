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
  // Requires a nodal Lagrange basis (shape index i == local node i, so n == n_nodes());
  // Assembly::reinitDual guarantees this before calling.

  // Standard shapes phi[i][qp]; all_shapes fills a pre-sized container, so size it first.
  std::vector<std::vector<Real>> phi(n, std::vector<Real>(pts.size()));
  FEInterface::all_shapes(dim, fe_type, &elem, pts, phi);

  // Standard mass matrix A(i,j) = integral(phi_i phi_j) and diagonal d(i) = integral(phi_i), using
  // the same (pts, JxW) as the untransformed dual so that T = I reproduces it exactly.
  DenseMatrix<Real> A(n, n);
  DenseVector<Real> d(n);
  for (const auto i : make_range(n))
    for (const auto qp : index_range(JxW))
    {
      d(i) += JxW[qp] * phi[i][qp];
      for (const auto j : make_range(n))
        A(i, j) += JxW[qp] * phi[i][qp] * phi[j][qp];
    }

  // Locally-quadratic transform Ntilde = T N; preserves the partition of unity so the dual
  // reproduces constants. alpha = 1/5 (Popp et al., SIAM J. Sci. Comput. 34(4):B421-B446, 2012,
  // Sec. 4.4.1) keeps the transformed diagonal positive (QUAD8 -> 1/5, 4/5; TRI6 -> 1/15, 1/10);
  // do not change it without re-deriving that positivity.
  const Real alpha = 1.0 / 5.0;
  DenseMatrix<Real> T(n, n);
  for (const auto i : make_range(n))
    T(i, i) = 1.0;
  // Mid-edge nodes are the trailing indices [n_vertices(), n_nodes()). Each scales its own column
  // by (1 - 2 alpha) and adds alpha in its adjacent vertex rows; vertex columns stay identity.
  const unsigned int n_vertices = elem.n_vertices();
  for (const auto m : make_range(n_vertices, n))
  {
    T(m, m) = 1.0 - 2.0 * alpha;
    for (const auto v : make_range(elem.n_second_order_adjacent_vertices(m)))
      T(elem.second_order_adjacent_vertex(m, v), m) += alpha;
  }

  // dtilde = T d (strictly positive on a full face; partial or edge-dropped coverage integrates
  // only covered sub-segments, where positivity is not guaranteed).
  DenseVector<Real> dtilde(n);
  T.vector_mult(dtilde, d);

  // dual_coeff = A^-1 T^-1 diag(dtilde), one column at a time: column j is dtilde(j) * (T^-1 e_j),
  // back-solved through A (SPD). lu_solve/cholesky_solve factorize T and A in place on the first
  // column and reuse them; dtilde is formed above before lu_solve factorizes T.
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
