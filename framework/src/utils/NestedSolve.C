//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NestedSolve.h"

#include "libmesh/utility.h"

InputParameters
NestedSolve::validParams()
{
  InputParameters params = emptyInputParameters();

  // Newton iteration control parameters
  params.addParam<Real>("relative_tolerance",
                        NestedSolve::relativeToleranceDefault(),
                        "Relative convergence tolerance for Newton iteration");
  params.addParam<Real>("absolute_tolerance",
                        NestedSolve::absoluteToleranceDefault(),
                        "Absolute convergence tolerance for Newton iteration");
  params.addParam<unsigned int>(
      "min_iterations",
      NestedSolve::minIterationsDefault(),
      "Minimum number of non linear iterations to execute before accepting convergence");
  params.addParam<unsigned int>("max_iterations",
                                NestedSolve::maxIterationsDefault(),
                                "Maximum number of non linear iterations");
  return params;
}

NestedSolve::NestedSolve()
  : _relative_tolerance_square(Utility::pow<2>(relativeToleranceDefault())),
    _absolute_tolerance_square(Utility::pow<2>(absoluteToleranceDefault())),
    _min_iterations(minIterationsDefault()),
    _max_iterations(maxIterationsDefault()),
    _state(State::NONE),
    _n_iterations(0)
{
}

NestedSolve::NestedSolve(const InputParameters & params)
  : _relative_tolerance_square(Utility::pow<2>(params.get<Real>("relative_tolerance"))),
    _absolute_tolerance_square(Utility::pow<2>(params.get<Real>("absolute_tolerance"))),
    _min_iterations(params.get<unsigned int>("min_iterations")),
    _max_iterations(params.get<unsigned int>("max_iterations")),
    _state(State::NONE),
    _n_iterations(0)
{
}

void
NestedSolve::sizeItems(const NestedSolve::DynamicVector & guess,
                       NestedSolve::DynamicVector & residual,
                       NestedSolve::DynamicMatrix & jacobian) const
{
  const auto N = guess.size();
  residual.resize(N, 1);
  jacobian.resize(N, N);
}

void
NestedSolve::linear(RankTwoTensor A, RealVectorValue & x, RealVectorValue b) const
{
  x = A.inverse() * b;
}

Real
NestedSolve::normSquare(RealVectorValue v) const
{
  return v.norm_sq();
}
