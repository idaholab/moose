//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class BuildArrayVariableAux : public ArrayAuxKernel
{
public:
  static InputParameters validParams();

  BuildArrayVariableAux(const InputParameters & parameters);

protected:
  virtual void compute() override final;

  // Not used. The actual implementation is in compute().
  virtual RealEigenVector computeValue() override final { mooseError("Unused"); }

  const std::vector<const VariableValue *> _component_dofs;
};
