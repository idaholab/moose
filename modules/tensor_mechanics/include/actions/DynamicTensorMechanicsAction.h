//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
