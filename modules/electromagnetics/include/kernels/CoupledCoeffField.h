//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

class CoupledCoeffField : public ADKernel
{
public:
  static InputParameters validParams();

  CoupledCoeffField(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  Real _coefficient;

  const Function & _func;

  const ADVariableValue & _coupled_val;

  Real _sign;
};
