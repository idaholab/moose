//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SymmetricRankTwoTensorImplementation.h"

template <>
void
SymmetricRankTwoTensor::syev(const char * calculation_type,
                             std::vector<Real> & eigvals,
                             std::vector<Real> & a) const
{
  eigvals.resize(Ndim);
  a.resize(Ndim * Ndim);

  // prepare data for the LAPACKsyev_ routine (which comes from petscblaslapack.h)
  PetscBLASInt nd = Ndim;
  PetscBLASInt lwork = 66 * nd;
  PetscBLASInt info;
  std::vector<PetscScalar> work(lwork);

  auto A = RankTwoTensor(*this);
  for (auto i : make_range(Ndim))
    for (auto j : make_range(Ndim))
      // a is destroyed by dsyev, and if calculation_type == "V" then eigenvectors are placed
      // there
      a[i * Ndim + j] = A(i, j);

  // compute the eigenvalues only (if calculation_type == "N"),
  // or both the eigenvalues and eigenvectors (if calculation_type == "V")
  // assume upper triangle of a is stored (second "U")
  LAPACKsyev_(calculation_type, "U", &nd, &a[0], &nd, &eigvals[0], &work[0], &lwork, &info);

  if (info != 0)
    mooseError("In computing the eigenvalues and eigenvectors for the symmetric rank-2 tensor (",
               Moose::stringify(a),
               "), the PETSC LAPACK syev routine returned error code ",
               info);
}

template <>
void
SymmetricRankTwoTensor::symmetricEigenvalues(std::vector<Real> & eigvals) const
{
  std::vector<Real> a;
  syev("N", eigvals, a);
}

template <>
void
SymmetricRankTwoTensor::symmetricEigenvaluesEigenvectors(std::vector<Real> & eigvals,
                                                         RankTwoTensor & eigvecs) const
{
  std::vector<Real> a;
  syev("V", eigvals, a);

  for (auto i : make_range(Ndim))
    for (auto j : make_range(Ndim))
      eigvecs(j, i) = a[i * Ndim + j];
}
