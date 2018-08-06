//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceUOPPS.h"
#include "InterfaceUO.h"

registerMooseObject("MooseTestApp", InterfaceUOPPS);

template <>
InputParameters
validParams<InterfaceUOPPS>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<UserObjectName>("user_object", "The name of the user object");
  params.addParam<bool>(
      "return_var_jump",
      false,
      "if true the user object returns the jump of the variable at the interface, if false "
      "(default) the average material property accross the interface");

  return params;
}

InterfaceUOPPS::InterfaceUOPPS(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _uo(getUserObject<InterfaceUO>("user_object")),
    _mean_mat_prop(0.),
    _mean_var_jump(0.)
{
}

InterfaceUOPPS::~InterfaceUOPPS() {}

void
InterfaceUOPPS::initialize()
{
  _mean_mat_prop = 0;
  _mean_var_jump = 0;
}

void
InterfaceUOPPS::execute()
{
  _mean_mat_prop = _uo.getMeanMatProp();
  _mean_var_jump = _uo.getMeanVarJump();
}

Real
InterfaceUOPPS::getMeanMatProp()
{
  return _mean_mat_prop;
}

Real
InterfaceUOPPS::getMeanVarJump()
{
  return _mean_var_jump;
}

Real
InterfaceUOPPS::getValue()
{
  if (!getParam<bool>("return_var_jump"))
    return getMeanMatProp();
  else
    return getMeanVarJump();
}
