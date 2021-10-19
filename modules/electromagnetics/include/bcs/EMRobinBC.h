//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

class EMRobinBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  EMRobinBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  const ADVariableValue & _field_real;

  const ADVariableValue & _field_imag;

  const MooseEnum _component;

  const Function & _func_real;

  const Function & _func_imag;

  const Function & _profile_func_real;

  const Function & _profile_func_imag;

  Real _coeff_real;

  Real _coeff_imag;

  Real _sign;

  const MooseEnum _mode;
};
