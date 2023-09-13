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
 * Coupled Double well Potential with a prefactor
 *psi * (psi^2 - 1)
 */
class PhaseFieldCoupledDoubleWellPotential : public ADKernelValue
{
public:
  static InputParameters validParams();  
  PhaseFieldCoupledDoubleWellPotential(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

  const ADVariableValue & _c;
  const Real & _prefactor;

};
