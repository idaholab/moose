//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SteadyStateRelativeChangeNorm.h"

registerMooseObject("MooseApp", SteadyStateRelativeChangeNorm);

InputParameters
SteadyStateRelativeChangeNorm::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addClassDescription(
      "Returns the relative change in the solution from the previous solve");
  return params;
}

SteadyStateRelativeChangeNorm::SteadyStateRelativeChangeNorm(const InputParameters & parameters)
  : GeneralPostprocessor(parameters)
{
  _transient_executioner = dynamic_cast<Transient *>(_app.getExecutioner());
  if (!_transient_executioner)
    mooseError("This postprocessor can only be used with Transient-derived executioners!");
}

Real
SteadyStateRelativeChangeNorm::getValue()
{
  return _transient_executioner->getSolutionChangeNorm();
}
