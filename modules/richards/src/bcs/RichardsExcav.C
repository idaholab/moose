/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "RichardsExcav.h"
#include "Function.h"

#include <iostream>

template <>
InputParameters
validParams<RichardsExcav>()
{
  InputParameters params = validParams<NodalBC>();
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
