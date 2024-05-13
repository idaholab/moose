//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DenseMatrix.h"
#include "ADReal.h"
#include "MooseError.h"

#include "libmesh/dense_matrix_base_impl.h"
#include "libmesh/dense_matrix_impl.h"

namespace libMesh
{
template <>
void
DenseMatrix<ADReal>::_multiply_blas(const DenseMatrixBase<ADReal> &, _BLAS_Multiply_Flag)
{
  mooseError("No blas/lapack support for type ", demangle(typeid(ADReal).name()));
}

template <>
void
DenseMatrix<ADReal>::_lu_decompose_lapack()
{
  mooseError("No blas/lapack support for type ", demangle(typeid(ADReal).name()));
}

template <>
void
DenseMatrix<ADReal>::_svd_lapack(DenseVector<Real> &)
{
  mooseError("No blas/lapack support for type ", demangle(typeid(ADReal).name()));
}

template <>
void
DenseMatrix<ADReal>::_svd_lapack(DenseVector<Real> &, DenseMatrix<Number> &, DenseMatrix<Number> &)
{
  mooseError("No blas/lapack support for type ", demangle(typeid(ADReal).name()));
}

template <>
void
DenseMatrix<ADReal>::_svd_helper(
    char, char, std::vector<Real> &, std::vector<Number> &, std::vector<Number> &)
{
  mooseError("No blas/lapack support for type ", demangle(typeid(ADReal).name()));
}

template <>
void
DenseMatrix<ADReal>::_svd_solve_lapack(const DenseVector<ADReal> & /*rhs*/,
                                       DenseVector<ADReal> & /*x*/,
                                       Real /*rcond*/) const
{
  mooseError("No blas/lapack support for type ", demangle(typeid(ADReal).name()));
}

template <>
void
DenseMatrix<ADReal>::_evd_lapack(DenseVector<ADReal> &,
                                 DenseVector<ADReal> &,
                                 DenseMatrix<ADReal> *,
                                 DenseMatrix<ADReal> *)
{
  mooseError("No blas/lapack support for type ", demangle(typeid(ADReal).name()));
}

template <>
void
DenseMatrix<ADReal>::_lu_back_substitute_lapack(const DenseVector<ADReal> &, DenseVector<ADReal> &)
{
  mooseError("No blas/lapack support for type ", demangle(typeid(ADReal).name()));
}

template <>
void
DenseMatrix<ADReal>::_matvec_blas(
    ADReal, ADReal, DenseVector<ADReal> &, const DenseVector<ADReal> &, bool) const
{
  mooseError("No blas/lapack support for type ", demangle(typeid(ADReal).name()));
}

template <>
DenseMatrix<ADReal>::DenseMatrix(const unsigned int new_m, const unsigned int new_n)
  : DenseMatrixBase<ADReal>(new_m, new_n), use_blas_lapack(false), _val(), _decomposition_type(NONE)
{
  this->resize(new_m, new_n);
}

template class DenseMatrixBase<ADReal>;
template class DenseMatrix<ADReal>;

template void DenseMatrix<ADReal>::vector_mult_add(DenseVector<ADReal> &,
                                                   const int,
                                                   const DenseVector<ADReal> &) const;
template void DenseMatrix<ADReal>::vector_mult_add(DenseVector<ADReal> &,
                                                   const double,
                                                   const DenseVector<ADReal> &) const;

template void DenseMatrix<Real>::vector_mult(DenseVector<ADReal> &,
                                             const DenseVector<ADReal> &) const;
template void DenseMatrix<ADReal>::vector_mult(DenseVector<ADReal> &,
                                               const DenseVector<Real> &) const;

template void DenseMatrix<ADReal>::cholesky_solve(const DenseVector<ADReal> & b,
                                                  DenseVector<ADReal> & x);
}
