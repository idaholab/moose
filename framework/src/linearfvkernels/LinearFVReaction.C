//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVReaction.h"
#include "Assembly.h"
#include "SubProblem.h"

registerMooseObject("MooseApp", LinearFVReaction);

InputParameters
LinearFVReaction::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription(
      "Represents the matrix and right hand side contributions of a reaction "
      "term ($c u$) in a partial differential equation.");
  params.addParam<MooseFunctorName>("coeff", 1.0, "The reaction coefficient.");
  return params;
}

LinearFVReaction::LinearFVReaction(const InputParameters & params)
  : LinearFVElementalKernel(params), _coefficient(getFunctor<Real>("coeff"))
{
}

Real
LinearFVReaction::computeMatrixContribution()
{
  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  return _coefficient(elem_arg, determineState()) * _current_elem_info->volume() *
         _current_elem_info->coordFactor();
}

Real
LinearFVReaction::computeRightHandSideContribution()
{
  return 0.0;
}
