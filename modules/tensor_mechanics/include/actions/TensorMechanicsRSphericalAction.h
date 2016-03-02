/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSRSPHERICALACTION_H
#define TENSORMECHANICSRSPHERICALACTION_H

#include "Action.h"

class TensorMechanicsRSphericalAction;

template<>
InputParameters validParams<TensorMechanicsRSphericalAction>();

class TensorMechanicsRSphericalAction : public Action
{
public:
  TensorMechanicsRSphericalAction(const InputParameters & params);

  virtual void act();
};

#endif //TENSORMECHANICSRSPHERICALACTION_H
