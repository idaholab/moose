/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsRSphericalAction.h"

template<>
InputParameters validParams<TensorMechanicsRSphericalAction>()
{
  return validParams<TensorMechanicsAction>();
}

TensorMechanicsRSphericalAction::TensorMechanicsRSphericalAction(const InputParameters & params) :
    TensorMechanicsAction(params)
{
  mooseDeprecated("Use the 'TensorMechanics' action instead. It autodetects the coordinate system.");
}
