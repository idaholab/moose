//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVReactionKernel.h"
#include "Assembly.h"
#include "SubProblem.h"

InputParameters
LinearFVReactionKernel::validParams()
{
  InputParameters params = ElementalLinearFVKernel::validParams();
  params.addParam<Real>("coeff", 1.0, "The reaction coefficient.");
  return params;
}

LinearFVReactionKernel::LinearFVReactionKernel(const InputParameters & params)
  : ElementalLinearFVKernel(params), _coeff(getParam<Real>("coeff"))
{
}

Real
LinearFVReactionKernel::computeMatrixContribution()
{
  return _coeff * _current_elem_info->volume();
}

Real
LinearFVReactionKernel::computeRightHandSideContribution()
{
  return 0.0;
}
