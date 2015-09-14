/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DYNAMICTENSORMECHANICSACTION_H
#define DYNAMICTENSORMECHANICSACTION_H

#include "TensorMechanicsAction.h"

class DynamicTensorMechanicsAction;

template<>
InputParameters validParams<DynamicTensorMechanicsAction>();

class DynamicTensorMechanicsAction : public TensorMechanicsAction
{
public:
  DynamicTensorMechanicsAction(const InputParameters & params);

  virtual void act();
  virtual void addkernel(const std::string & name, InputParameters & params);
};

#endif //DYNAMICTENSORMECHANICSACTION_H
