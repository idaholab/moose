//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MDFluidKernelStabilization.h"

// The spatial part of the 3D momentum conservation for fluid flow
class MDFluidMomentumKernel : public MDFluidKernelStabilization
{
public:
  static InputParameters validParams();

  MDFluidMomentumKernel(const InputParameters & parameters);
  virtual ~MDFluidMomentumKernel() {}

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const VariableGradient & _grad_eps;
  bool _conservative_form;
  unsigned _component;
};
