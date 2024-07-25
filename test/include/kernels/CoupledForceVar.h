//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

/**
 * CoupledForceVar using a variable coupled through its name directly
 */
class CoupledForceVar : public Kernel
{
public:
  static InputParameters validParams();

  CoupledForceVar(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const std::map<std::string, Real> & _var_coef;
  std::vector<const VariableValue *> _vars;
  std::vector<Real> _coefs;
};
