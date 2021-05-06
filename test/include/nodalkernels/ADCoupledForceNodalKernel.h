//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADNodalKernel.h"

/**
 * Adds a force proportional to the value of the coupled variable
 */
class ADCoupledForceNodalKernel : public ADNodalKernel
{
public:
  static InputParameters validParams();

  ADCoupledForceNodalKernel(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

private:
  /// The number of the coupled variable
  const unsigned int _v_var;

  /// The value of the coupled variable
  const ADVariableValue & _v;

  /// A multiplicative factor for computing the coupled force
  const Real _coef;
};
