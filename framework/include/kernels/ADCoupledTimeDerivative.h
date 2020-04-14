//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 * This calculates the time derivative for a coupled variable
 **/
class ADCoupledTimeDerivative : public ADKernelValue
{
public:
  static InputParameters validParams();

  ADCoupledTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

  const ADVariableValue & _v_dot;
};
