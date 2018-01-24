//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialTimeStepPostprocessor.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"

#include "libmesh/quadrature.h"

#include <algorithm>
#include <limits>

template <>
InputParameters
validParams<MaterialTimeStepPostprocessor>()
{
  InputParameters params = validParams<ElementPostprocessor>();

  params.addClassDescription("This postprocessor estimates a timestep that reduces the increment "
                             "change in a aux variable below a given threshold.");

  return params;
}

MaterialTimeStepPostprocessor::MaterialTimeStepPostprocessor(const InputParameters & parameters)
  : ElementPostprocessor(parameters),
    _matl_time_step(getMaterialPropertyByName<Real>("matl_timestep_limit"))
{
}

void
MaterialTimeStepPostprocessor::initialize()
{
  _value = std::numeric_limits<Real>::max(); // start w/ the min
}

void
MaterialTimeStepPostprocessor::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    _value = std::min(_value, _matl_time_step[_qp]);
}

Real
MaterialTimeStepPostprocessor::getValue()
{
  gatherMin(_value);
  return _value;
}

void
MaterialTimeStepPostprocessor::threadJoin(const UserObject & y)
{
  const MaterialTimeStepPostprocessor & pps = static_cast<const MaterialTimeStepPostprocessor &>(y);
  _value = std::min(_value, pps._value);
}
