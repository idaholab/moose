//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVSourceKernel.h"
#include "Assembly.h"
#include "SubProblem.h"

registerMooseObject("MooseApp", LinearFVSourceKernel);

InputParameters
LinearFVSourceKernel::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription(
      "Represents the matrix and right hand side contributions of a "
      "solution-independent source term in a partial differential equation.");
  params.addParam<MooseFunctorName>("source_density", 1.0, "The source density.");
  return params;
}

LinearFVSourceKernel::LinearFVSourceKernel(const InputParameters & params)
  : LinearFVElementalKernel(params), _source_density(getFunctor<Real>("source_density"))
{
}

Real
LinearFVSourceKernel::computeMatrixContribution()
{
  return 0.0;
}

Real
LinearFVSourceKernel::computeRightHandSideContribution()
{
  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  return _source_density(elem_arg, determineState()) * _current_elem_info->volume();
}
