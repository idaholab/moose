//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

class ADPhaseFieldTwoPhaseSurfaceTension : public ADVectorKernelValue
{
public:
  static InputParameters validParams();
  ADPhaseFieldTwoPhaseSurfaceTension(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  const ADVariableValue & _auxpf;
  const ADVariableGradient & _grad_pf;
  const Real & _coeff;
};
