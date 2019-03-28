//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseTypes.h"
#include "libmesh/elem.h"
#include "libmesh/dense_matrix_base_impl.h"
#include "libmesh/dense_matrix_impl.h"
#include "metaphysicl/numberarray.h"
#include "metaphysicl/dualnumber.h"

namespace Moose
{
const SubdomainID ANY_BLOCK_ID = libMesh::Elem::invalid_subdomain_id - 1;
const SubdomainID INVALID_BLOCK_ID = libMesh::Elem::invalid_subdomain_id;
const BoundaryID ANY_BOUNDARY_ID = static_cast<BoundaryID>(-1);
const BoundaryID INVALID_BOUNDARY_ID = libMesh::BoundaryInfo::invalid_id;
}

// bare bones explicit instantiations for DenseMatrix<DualReal>
namespace libMesh
{
template class DenseMatrixBase<DualReal>;
template class DenseMatrix<DualReal>;
template void
DenseMatrix<Real>::vector_mult(DenseVector<CompareTypes<Real, DualReal>::supertype> & dest,
                               const DenseVector<DualReal> & arg) const;
}
