//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankTwoTensorImplementation.h"

template <>
void
RankTwoTensor::printReal(std::ostream & stm) const
{
  this->print(stm);
}

template <>
void
ADRankTwoTensor::printReal(std::ostream & stm) const
{
  const ADRankTwoTensor & a = *this;
  for (const auto i : make_range(N))
  {
    for (const auto j : make_range(N))
      stm << std::setw(15) << a(i, j).value() << ' ';
    stm << std::endl;
  }
}

template <>
void
ADRankTwoTensor::printADReal(unsigned int nDual, std::ostream & stm) const
{
  const ADRankTwoTensor & a = *this;
  for (const auto i : make_range(N))
  {
    for (const auto j : make_range(N))
    {
      stm << std::setw(15) << a(i, j).value() << " {";
      for (const auto k : make_range(nDual))
        stm << std::setw(5) << a(i, j).derivatives()[k] << ' ';
      stm << " }";
    }
    stm << std::endl;
  }
}

template <>
void
RankTwoTensor::syev(const char * calculation_type,
                    std::vector<Real> & eigvals,
                    std::vector<Real> & a) const
{
  eigvals.resize(N);
  a.resize(N * N);

  // prepare data for the LAPACKsyev_ routine (which comes from petscblaslapack.h)
  PetscBLASInt nd = N;
  PetscBLASInt lwork = 66 * nd;
  PetscBLASInt info;
  std::vector<PetscScalar> work(lwork);

  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      // a is destroyed by dsyev, and if calculation_type == "V" then eigenvectors are placed
      // there Note the explicit symmeterisation
      a[i * N + j] = 0.5 * (this->operator()(i, j) + this->operator()(j, i));

  // compute the eigenvalues only (if calculation_type == "N"),
  // or both the eigenvalues and eigenvectors (if calculation_type == "V")
  // assume upper triangle of a is stored (second "U")
  LAPACKsyev_(calculation_type, "U", &nd, &a[0], &nd, &eigvals[0], &work[0], &lwork, &info);

  if (info != 0)
    mooseException(
        "In computing the eigenvalues and eigenvectors for the symmetric rank-2 tensor (",
        Moose::stringify(a),
        "), the PETSC LAPACK syev routine returned error code ",
        info);
}

template <>
void
RankTwoTensor::symmetricEigenvalues(std::vector<Real> & eigvals) const
{
  std::vector<Real> a;
  syev("N", eigvals, a);
}

template <>
void
RankTwoTensor::symmetricEigenvaluesEigenvectors(std::vector<Real> & eigvals,
                                                RankTwoTensor & eigvecs) const
{
  std::vector<Real> a;
  syev("V", eigvals, a);

  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      eigvecs(j, i) = a[i * N + j];
}

template <>
void
ADRankTwoTensor::symmetricEigenvaluesEigenvectors(std::vector<ADReal> & eigvals,
                                                  ADRankTwoTensor & eigvecs) const
{
  typedef Eigen::Matrix<ADReal, N, N> RankTwoMatrix;
  RankTwoMatrix self;
  for (const auto i : make_range(N))
    for (unsigned int j = i; j < N; ++j)
    {
      auto & v = self(j, i);
      v = (*this)(i, j);
      if (i != j && MooseUtils::absoluteFuzzyEqual(v, 0.0))
        v.value() = 0.0;
    }

  Eigen::SelfAdjointEigenSolver<RankTwoMatrix> es;
  es.compute(self);

  const auto & lambda = es.eigenvalues();
  eigvals.resize(N);
  for (const auto i : make_range(N))
    eigvals[i] = lambda(i);

  const auto & v = es.eigenvectors();
  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      eigvecs(i, j) = v(i, j);
}

template <>
void
RankTwoTensor::getRUDecompositionRotation(RankTwoTensor & rot) const
{
  const RankTwoTensor & a = *this;
  RankTwoTensor c, diag, evec;
  PetscScalar cmat[N][N], work[10];
  PetscReal w[N];

  // prepare data for the LAPACKsyev_ routine (which comes from petscblaslapack.h)
  PetscBLASInt nd = N, lwork = 10, info;

  c = a.transpose() * a;

  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      cmat[i][j] = c(i, j);

  LAPACKsyev_("V", "U", &nd, &cmat[0][0], &nd, w, work, &lwork, &info);

  if (info != 0)
    mooseException(
        "In computing the eigenvalues and eigenvectors of a symmetric rank-2 tensor, the "
        "PETSC LAPACK syev routine returned error code ",
        info);

  diag.zero();

  for (const auto i : make_range(N))
    diag(i, i) = std::sqrt(w[i]);

  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      evec(i, j) = cmat[i][j];

  rot = a * (evec.transpose() * diag * evec).inverse();
}
