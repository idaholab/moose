//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LeastSquaresFitBase.h"
#include "MooseError.h"

#include "libmesh/int_range.h"
// Ignore warnings from Eigen related to deprecated declarations (C++17)
#include "libmesh/ignore_warnings.h"
#include <Eigen/Dense>
#include "libmesh/restore_warnings.h"

LeastSquaresFitBase::LeastSquaresFitBase() {}

LeastSquaresFitBase::LeastSquaresFitBase(const std::vector<Real> & x, const std::vector<Real> & y)
  : _x(x), _y(y)
{
}

void
LeastSquaresFitBase::setVariables(const std::vector<Real> & x, const std::vector<Real> & y)
{
  _x = x;
  _y = y;
}

void
LeastSquaresFitBase::generate()
{
  if (_x.empty())
    mooseError("Empty variables in LeastSquaresFitBase. x and y must be set in the constructor or "
               "using setVariables(x, y)");

  fillMatrix();
  doLeastSquares();
}

void
LeastSquaresFitBase::doLeastSquares()
{
  _coeffs.resize(_num_coeff);

  typedef Eigen::Matrix<Real, Eigen::Dynamic, 1> SolveVec;
  auto b = Eigen::Map<SolveVec, Eigen::Unaligned>(_y.data(), _y.size());
  auto x = Eigen::Map<SolveVec, Eigen::Unaligned>(_coeffs.data(), _num_coeff);
  typedef Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> SolveMatrix;
  auto A = Eigen::Map<SolveMatrix, Eigen::Unaligned>(_matrix.data(), _y.size(), _num_coeff);
  x = A.colPivHouseholderQr().solve(b);
}

unsigned int
LeastSquaresFitBase::getSampleSize()
{
  return _x.size();
}

const std::vector<Real> &
LeastSquaresFitBase::getCoefficients()
{
  return _coeffs;
}
