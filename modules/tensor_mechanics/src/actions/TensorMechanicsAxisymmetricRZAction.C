/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsAxisymmetricRZAction.h"

template<>
InputParameters validParams<TensorMechanicsAxisymmetricRZAction>()
{
  return validParams<TensorMechanicsAction>();
}

TensorMechanicsAxisymmetricRZAction::TensorMechanicsAxisymmetricRZAction(const InputParameters & params) :
    TensorMechanicsAction(params)
{
  mooseDeprecated("Use the 'TensorMechanics' action instead. It autodetects the coordinate system.");
}
