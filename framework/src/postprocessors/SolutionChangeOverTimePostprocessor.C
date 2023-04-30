//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolutionChangeOverTimePostprocessor.h"

registerMooseObject("MooseApp", SolutionChangeOverTimePostprocessor);

InputParameters
SolutionChangeOverTimePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addClassDescription(
      "Returns the relative change in the solution between the two previous time steps");
  return params;
}

SolutionChangeOverTimePostprocessor::SolutionChangeOverTimePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters)
{
  _transient_executioner = dynamic_cast<Transient *>(_app.getExecutioner());
  if (!_transient_executioner)
    mooseError("This postprocessor can only be used with Transient-derived executioners!");
}

Real
SolutionChangeOverTimePostprocessor::getValue()
{
  return _transient_executioner->getSolutionChangeNorm();
}
