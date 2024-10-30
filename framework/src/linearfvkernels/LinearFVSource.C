//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVSource.h"
#include "Assembly.h"
#include "SubProblem.h"

registerMooseObject("MooseApp", LinearFVSource);

InputParameters
LinearFVSource::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription(
      "Represents the matrix and right hand side contributions of a "
      "solution-independent source term in a partial differential equation.");
  params.addParam<MooseFunctorName>("source_density", 1.0, "The source density.");
  params.addParam<Real>("scaling_factor", 1.0, "Coefficient to multiply the body force term with.");
  return params;
}

LinearFVSource::LinearFVSource(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _source_density(getFunctor<Real>("source_density")),
    _scale(getParam<Real>("scaling_factor"))
{
}

Real
LinearFVSource::computeMatrixContribution()
{
  // This doesn't contribute to the matrix as we assumed it was independent of
  // the solution
  return 0.0;
}

Real
LinearFVSource::computeRightHandSideContribution()
{
  // The contribution to the right hand side is s_C*V_C
  return _scale * _source_density(makeElemArg(_current_elem_info->elem()), determineState()) *
         _current_elem_volume;
}
