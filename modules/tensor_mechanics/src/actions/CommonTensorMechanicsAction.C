/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CommonTensorMechanicsAction.h"
#include "TensorMechanicsAction.h"

template <>
InputParameters
validParams<CommonTensorMechanicsAction>()
{
  InputParameters params = validParams<TensorMechanicsActionBase>();
  return params;
}

CommonTensorMechanicsAction::CommonTensorMechanicsAction(const InputParameters & parameters)
  : Action(parameters)
{
}
