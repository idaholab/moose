//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 * Implements a source term proportional to the value of a coupled variable. Weak form: $(\\psi_i,
 * -\\sigma v)$.
 */
class ADCoupledForce : public ADKernel
{
public:
  static InputParameters validParams();

  ADCoupledForce(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  /// Coupled variable number
  unsigned int _v_var;
  /// Coupled variable
  const ADVariableValue & _v;
  /// Multiplier for the coupled force term
  Real _coef;
};
