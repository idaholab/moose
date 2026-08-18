//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/dense_matrix.h"
#include "libmesh/fe_type.h"
#include "libmesh/point.h"
#include "libmesh/enum_elem_type.h"

#include <vector>

namespace libMesh
{
class Elem;
}

namespace Moose
{
namespace Mortar
{
/**
 * Whether the Popp et al. (2012) locally-quadratic transformed dual (biorthogonal) basis is
 * defined for a lower-dimensional face element type. It is only needed for second-order Lagrange
 * faces whose standard dual diagonal is non-positive: TRI6 vertices integrate to zero and QUAD8
 * corners to -1/3, which makes the per-node dual normalization ill-posed. QUAD9 is deliberately
 * excluded because its standard dual diagonal is already positive (and its center node has four
 * adjacent vertices, outside the mid-edge construction). All other types keep the standard dual.
 */
bool transformedDualBasisSupported(libMesh::ElemType elem_type);

/**
 * Compute the Popp et al. (2012) transformed dual shape-function coefficients for a second-order
 * Lagrange trace basis on a TRI6 or QUAD8 face element.
 *
 * The transformed dual shapes are expressed in the standard nodal basis as
 * dual_phi_j = sum_i dual_coeff(i, j) phi_i and are biorthogonal to the transformed primal basis
 * Ntilde = T N with a strictly positive diagonal dtilde = T d (d_i = integral(phi_i)). In libMesh
 * (row = standard node k, column = dual index j) indexing this is dual_coeff = A^-1 B with
 * A(i, j) = integral(phi_i phi_j) and B = T^-1 diag(T d); T = I reproduces libMesh's standard dual
 * construction exactly.
 *
 * @param elem the lower-dimensional face element (must be TRI6 or QUAD8)
 * @param fe_type the trace basis FE type (second-order Lagrange)
 * @param pts reference points on elem at which the integration weights are given
 * @param JxW integration weights (mortar-segment JxW) matching pts one-to-one
 * @param dual_coeff (output) n-by-n coefficient matrix, n = number of trace shapes
 */
void computeTransformedDualCoeffs(const libMesh::Elem & elem,
                                  const libMesh::FEType & fe_type,
                                  const std::vector<libMesh::Point> & pts,
                                  const std::vector<libMesh::Real> & JxW,
                                  libMesh::DenseMatrix<libMesh::Real> & dual_coeff);
}
}
