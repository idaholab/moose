//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ArrayKernel.h"

/**
 * This calculates the time derivative for a coupled variable
 **/
class ArrayCoupledTimeDerivative : public ArrayKernel
{
public:
  static InputParameters validParams();

  ArrayCoupledTimeDerivative(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(RealEigenVector & residual) override;
  virtual RealEigenVector computeQpJacobian() override;
  virtual RealEigenMatrix computeQpOffDiagJacobian(const MooseVariableFEBase & jvar) override;

  /// The first temporal derivative of the coupled variable
  const ArrayVariableValue & _v_dot;

  /// The Jacobian of the time derivative of the coupled variable with respect to the coupled variable
  const VariableValue & _dv_dot;

  /// The number of the coupled variable
  const unsigned int _v_var;
};
