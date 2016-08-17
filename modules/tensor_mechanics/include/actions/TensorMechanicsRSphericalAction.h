/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSRSPHERICALACTION_H
#define TENSORMECHANICSRSPHERICALACTION_H

#include "TensorMechanicsAction.h"

class TensorMechanicsRSphericalAction;

template<>
InputParameters validParams<TensorMechanicsRSphericalAction>();

class TensorMechanicsRSphericalAction : public TensorMechanicsAction
{
public:
  TensorMechanicsRSphericalAction(const InputParameters & params);
};

#endif //TENSORMECHANICSRSPHERICALACTION_H
