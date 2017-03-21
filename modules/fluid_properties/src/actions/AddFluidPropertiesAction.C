/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AddFluidPropertiesAction.h"

template <>
InputParameters
validParams<AddFluidPropertiesAction>()
{
  return validParams<AddUserObjectAction>();
}

AddFluidPropertiesAction::AddFluidPropertiesAction(InputParameters params)
  : AddUserObjectAction(params)
{
}
