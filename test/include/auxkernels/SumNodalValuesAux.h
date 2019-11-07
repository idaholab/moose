//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxNodalScalarKernel.h"

class SumNodalValuesAux : public AuxNodalScalarKernel
{
public:
  static InputParameters validParams();

  SumNodalValuesAux(const InputParameters & parameters);
  virtual ~SumNodalValuesAux();

  virtual void compute();

protected:
  virtual Real computeValue();

  const VariableValue & _sum_var;
};
