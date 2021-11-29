//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalKernel.h"

/**
 * Adds a force proportional to the value of the coupled variable
 */
class CoupledForceNodalKernel : public NodalKernel
{
public:
  static InputParameters validParams();

  CoupledForceNodalKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  /// The number of the coupled variable
  const unsigned int _v_var;

  /// The value of the coupled variable
  const VariableValue & _v;

  /// A multiplicative factor for computing the coupled force
  const Real _coef;
};
