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

registerMooseObject("MooseApp", LinearFVReactionKernel);

InputParameters
LinearFVReactionKernel::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addParam<MooseFunctorName>("coeff", 1.0, "The reaction coefficient.");
  return params;
}

LinearFVReactionKernel::LinearFVReactionKernel(const InputParameters & params)
  : LinearFVElementalKernel(params), _coefficient(getFunctor<Real>("coeff"))
{
}

Real
LinearFVReactionKernel::computeMatrixContribution()
{
  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  return _coefficient(elem_arg, determineState()) * _current_elem_info->volume() *
         _current_elem_info->coordFactor();
}

Real
LinearFVReactionKernel::computeRightHandSideContribution()
{
  return 0.0;
}
