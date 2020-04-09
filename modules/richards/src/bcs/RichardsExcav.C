//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsExcav.h"
#include "Function.h"

#include <iostream>

registerMooseObject("RichardsApp", RichardsExcav);

InputParameters
RichardsExcav::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addRequiredParam<Real>(
      "p_excav",
      "Value of the variable at the surface of the excavation.  Eg atmospheric pressure");
  params.addRequiredParam<FunctionName>(
      "excav_geom_function",
      "The function describing the excavation geometry (type RichardsExcavGeom)");
  params.addClassDescription("Allows the user to set variable values at the face of an excavation. "
                             " You must have defined the excavation start time, start position, "
                             "etc, through the excav_geom_function");
  return params;
}

RichardsExcav::RichardsExcav(const InputParameters & parameters)
  : NodalBC(parameters),
    _p_excav(getParam<Real>("p_excav")),
    _func(getFunction("excav_geom_function"))
{
}

bool
RichardsExcav::shouldApply()
{
  if (_func.value(_t, *_current_node) == 0.0)
    return false;
  else
    return true;
}

Real
RichardsExcav::computeQpResidual()
{
  return _u[_qp] - _p_excav;
}
