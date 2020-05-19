//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NullKernel.h"

/**
 *This kernel is used to test that _phi_zero, _grad_phi_zero and _second_phi_zero help variables
 *have proper dimensions, i.e. [MaxVarNDofsPerElem][MaxQp]
 */
class PhiZeroKernel : public NullKernel
{
public:
  static InputParameters validParams();

  PhiZeroKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  const VariableSecond & _second_u;
  const VariablePhiSecond & _second_phi;
};
