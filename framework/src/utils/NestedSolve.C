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

template <bool is_ad>
InputParameters
NestedSolveTempl<is_ad>::validParams()
{
  InputParameters params = emptyInputParameters();

  // Newton iteration control parameters
  params.addParam<Real>("relative_tolerance",
                        relativeToleranceDefault(),
                        "Relative convergence tolerance for Newton iteration");
  params.addParam<Real>("absolute_tolerance",
                        absoluteToleranceDefault(),
                        "Absolute convergence tolerance for Newton iteration");
  params.addParam<Real>(
      "x_tolerance", xToleranceDefault(), "Threshold for minimum step size of linear iterations");
  params.addParam<unsigned int>(
      "min_iterations",
      minIterationsDefault(),
      "Minimum number of nonlinear iterations to execute before accepting convergence");
  params.addParam<unsigned int>(
      "max_iterations", maxIterationsDefault(), "Maximum number of nonlinear iterations");
  params.addParam<Real>("acceptable_multiplier",
                        acceptableMultiplierDefault(),
                        "Factor applied to relative and absolute "
                        "tolerance for acceptable nonlinear convergence if "
                        "iterations are no longer making progress");
  return params;
}

template <bool is_ad>
NestedSolveTempl<is_ad>::NestedSolveTempl()
  : _relative_tolerance_square(Utility::pow<2>(relativeToleranceDefault())),
    _absolute_tolerance_square(Utility::pow<2>(absoluteToleranceDefault())),
    _x_tolerance_square(Utility::pow<2>(xToleranceDefault())),
    _min_iterations(minIterationsDefault()),
    _max_iterations(maxIterationsDefault()),
    _acceptable_multiplier(acceptableMultiplierDefault()),
    _state(State::NONE),
    _n_iterations(0)
{
}

template <bool is_ad>
NestedSolveTempl<is_ad>::NestedSolveTempl(const InputParameters & params)
  : _relative_tolerance_square(Utility::pow<2>(params.get<Real>("relative_tolerance"))),
    _absolute_tolerance_square(Utility::pow<2>(params.get<Real>("absolute_tolerance"))),
    _x_tolerance_square(Utility::pow<2>(params.get<Real>("x_tolerance"))),
    _min_iterations(params.get<unsigned int>("min_iterations")),
    _max_iterations(params.get<unsigned int>("max_iterations")),
    _acceptable_multiplier(params.get<Real>("acceptable_multiplier")),
    _state(State::NONE),
    _n_iterations(0)
{
}

template <bool is_ad>
void
NestedSolveTempl<is_ad>::sizeItems(const NestedSolveTempl<is_ad>::DynamicVector & guess,
                                   NestedSolveTempl<is_ad>::DynamicVector & residual,
                                   NestedSolveTempl<is_ad>::DynamicMatrix & jacobian) const
{
  const auto N = guess.size();
  residual.resize(N, 1);
  jacobian.resize(N, N);
}

template <bool is_ad>
void
NestedSolveTempl<is_ad>::linear(const NSRankTwoTensor & A,
                                NSRealVectorValue & x,
                                const NSRealVectorValue & b) const
{
  x = A.inverse() * b;
}

template <bool is_ad>
Real
NestedSolveTempl<is_ad>::normSquare(const NSReal & v)
{
  return MetaPhysicL::raw_value(v) * MetaPhysicL::raw_value(v);
}

template <bool is_ad>
Real
NestedSolveTempl<is_ad>::normSquare(const NSRealVectorValue & v)
{
  return MetaPhysicL::raw_value(v) * MetaPhysicL::raw_value(v);
}

template class NestedSolveTempl<false>;
template class NestedSolveTempl<true>;
