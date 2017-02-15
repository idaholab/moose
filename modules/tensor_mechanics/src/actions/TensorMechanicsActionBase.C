/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsActionBase.h"
#include "CommonTensorMechanicsAction.h"
#include "ActionWarehouse.h"

template<>
InputParameters validParams<TensorMechanicsActionBase>()
{
  InputParameters params = validParams<Action>();
  return params;
}

TensorMechanicsActionBase::TensorMechanicsActionBase(const InputParameters & parameters) :
    Action(parameters)
{
  // check if a container block with common parameters is found
  auto action = _awh.getActions<CommonTensorMechanicsAction>();
  if (action.size() == 1)
    _pars.applyParameters(action[0]->parameters());
}
