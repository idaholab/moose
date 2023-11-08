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

InputParameters
LinearFVSourceKernel::validParams()
{
  InputParameters params = ElementalLinearFVKernel::validParams();
  params.addParam<Real>("source_density", 1.0, "The source density.");
  return params;
}

LinearFVSourceKernel::LinearFVSourceKernel(const InputParameters & params)
  : ElementalLinearFVKernel(params), _source_density(getParam<Real>("source_density"))
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
  return _source_density * _current_elem_info->volume();
}
