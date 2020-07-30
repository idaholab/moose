//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeKernel.h"

/**
 * Kernel describing porosity * d(concentration)/dt, where porosity is an AuxVariable.
 * This Kernel should not be used if porosity is time-dependent.
 * Mass lumping is employed for numerical stability
 */
class GeochemistryTimeDerivative : public TimeKernel
{
public:
  static InputParameters validParams();

  GeochemistryTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  const VariableValue & _nodal_u_dot;
  const VariableValue & _nodal_du_dot_du;
  const VariableValue & _porosity;
};
