//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVContinuityErrorSink.h"

#include "Assembly.h"
#include "SubProblem.h"

registerMooseObject("NavierStokesApp", LinearFVContinuityErrorSink);

InputParameters
LinearFVContinuityErrorSink::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription(
      "Linear FV kernel that mimics the reference-solver-style -Sp(a, phi) operator by treating the "
      "sink contribution implicitly and the opposite-sign source contribution explicitly.");
  params.addRequiredParam<MooseFunctorName>(
      "coeff",
      "The coefficient a in -Sp(a, variable). Positive values contribute to the matrix "
      "diagonal, while negative values are moved explicitly to the right hand side.");
  return params;
}

LinearFVContinuityErrorSink::LinearFVContinuityErrorSink(const InputParameters & params)
  : LinearFVElementalKernel(params), _coefficient(getFunctor<Real>("coeff"))
{
}

Real
LinearFVContinuityErrorSink::computeMatrixContribution()
{
  const auto coeff = _coefficient(makeElemArg(_current_elem_info->elem()), determineState());
  return std::max(coeff, 0.0) * _current_elem_volume;
}

Real
LinearFVContinuityErrorSink::computeRightHandSideContribution()
{
  const auto state = determineState();
  const auto coeff = _coefficient(makeElemArg(_current_elem_info->elem()), state);
  return std::max(-coeff, 0.0) * _var.getElemValue(*_current_elem_info, state) *
         _current_elem_volume;
}
