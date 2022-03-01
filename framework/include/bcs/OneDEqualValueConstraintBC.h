//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

/**
 * This is the \f$ \int \lambda dg\f$ term from the mortar method.
 * This can connect two 1D domains only.
 *
 * For higher dimensions, you should use face-face constraints.
 */
class OneDEqualValueConstraintBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  OneDEqualValueConstraintBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobianScalar(unsigned int jvar) override;

  const VariableValue & _lambda;
  unsigned int _lambda_var_number;
  unsigned int _component;
  Real _vg;
};
