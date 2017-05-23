/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
