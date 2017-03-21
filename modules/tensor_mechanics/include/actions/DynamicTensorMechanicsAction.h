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

template <>
InputParameters validParams<DynamicTensorMechanicsAction>();

class DynamicTensorMechanicsAction : public TensorMechanicsAction
{
public:
  DynamicTensorMechanicsAction(const InputParameters & params);

protected:
  virtual std::string getKernelType();
};

#endif // DYNAMICTENSORMECHANICSACTION_H
