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
 * Simple class to demonstrate off diagonal Jacobian contributions.
 */
class OptionallyCoupledForce : public Kernel
{
public:
  static InputParameters validParams();

  OptionallyCoupledForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  unsigned int _v_var;
  const VariableValue & _v;
  const VariableGradient & _grad_v;
  const VariableSecond & _second_v;
  const VariableValue & _v_dot;
  const VariableValue & _v_dot_du;
  bool _v_coupled;
};
