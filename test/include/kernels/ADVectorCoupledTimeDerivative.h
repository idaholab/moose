//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

#include "ADVectorKernel.h"

/**
 * Adds the time derivative of a coupled vector variable, (psi, dv/dt), to test that
 * adCoupledVectorDot() requests the AD vector shape data when it is the only use of v
 */
class ADVectorCoupledTimeDerivative : public ADVectorKernel
{
public:
  static InputParameters validParams();

  ADVectorCoupledTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// time derivative of the coupled vector variable
  const ADVectorVariableValue & _v_dot;
};
