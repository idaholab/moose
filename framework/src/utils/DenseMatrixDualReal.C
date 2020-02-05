//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DenseMatrix.h"
#include "DualReal.h"
#include "MooseError.h"

#include "libmesh/dense_matrix_base_impl.h"
#include "libmesh/dense_matrix_impl.h"

#include "DualRealOps.h"

namespace libMesh
{
template <>
void
DenseMatrix<DualReal>::_multiply_blas(const DenseMatrixBase<DualReal> &, _BLAS_Multiply_Flag)
{
  mooseError("No blas/lapack support for type ", demangle(typeid(DualReal).name()));
}

template <>
void
DenseMatrix<DualReal>::_lu_decompose_lapack()
{
  mooseError("No blas/lapack support for type ", demangle(typeid(DualReal).name()));
}

template <>
void
DenseMatrix<DualReal>::_svd_lapack(DenseVector<Real> &)
{
  mooseError("No blas/lapack support for type ", demangle(typeid(DualReal).name()));
}

template <>
void
DenseMatrix<DualReal>::_svd_lapack(DenseVector<Real> &,
                                   DenseMatrix<Number> &,
                                   DenseMatrix<Number> &)
{
  mooseError("No blas/lapack support for type ", demangle(typeid(DualReal).name()));
}

template <>
void
DenseMatrix<DualReal>::_svd_helper(
    char, char, std::vector<Real> &, std::vector<Number> &, std::vector<Number> &)
{
  mooseError("No blas/lapack support for type ", demangle(typeid(DualReal).name()));
}

template <>
void
DenseMatrix<DualReal>::_svd_solve_lapack(const DenseVector<DualReal> & /*rhs*/,
                                         DenseVector<DualReal> & /*x*/,
                                         Real /*rcond*/) const
{
  mooseError("No blas/lapack support for type ", demangle(typeid(DualReal).name()));
}

template <>
void
DenseMatrix<DualReal>::_evd_lapack(DenseVector<DualReal> &,
                                   DenseVector<DualReal> &,
                                   DenseMatrix<DualReal> *,
                                   DenseMatrix<DualReal> *)
{
  mooseError("No blas/lapack support for type ", demangle(typeid(DualReal).name()));
}

template <>
void
DenseMatrix<DualReal>::_lu_back_substitute_lapack(const DenseVector<DualReal> &,
                                                  DenseVector<DualReal> &)
{
  mooseError("No blas/lapack support for type ", demangle(typeid(DualReal).name()));
}

template <>
void
DenseMatrix<DualReal>::_matvec_blas(
    DualReal, DualReal, DenseVector<DualReal> &, const DenseVector<DualReal> &, bool) const
{
  mooseError("No blas/lapack support for type ", demangle(typeid(DualReal).name()));
}

template <>
DenseMatrix<DualReal>::DenseMatrix(const unsigned int new_m, const unsigned int new_n)
  : DenseMatrixBase<DualReal>(new_m, new_n),
    use_blas_lapack(false),
    _val(),
    _decomposition_type(NONE)
{
  this->resize(new_m, new_n);
}

template class DenseMatrixBase<DualReal>;
template class DenseMatrix<DualReal>;

template void DenseMatrix<DualReal>::vector_mult_add(DenseVector<DualReal> &,
                                                     const int,
                                                     const DenseVector<DualReal> &) const;
template void DenseMatrix<DualReal>::vector_mult_add(DenseVector<DualReal> &,
                                                     const double,
                                                     const DenseVector<DualReal> &) const;

template void DenseMatrix<Real>::vector_mult(DenseVector<DualReal> &,
                                             const DenseVector<DualReal> &) const;
template void DenseMatrix<DualReal>::vector_mult(DenseVector<DualReal> &,
                                                 const DenseVector<Real> &) const;

template void DenseMatrix<DualReal>::cholesky_solve(const DenseVector<DualReal> & b,
                                                    DenseVector<DualReal> & x);
}
