//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorKernel.h"

/**
 *  This calculates the time derivative for a coupled vector variable
 */
class VectorCoupledTimeDerivative : public VectorKernel
{
public:
  static InputParameters validParams();

  VectorCoupledTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const VectorVariableValue & _v_dot;
  const VariableValue & _dv_dot;
  const unsigned int _v_var;
};
