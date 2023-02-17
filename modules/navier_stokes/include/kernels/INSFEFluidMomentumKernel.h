//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFEFluidKernelStabilization.h"

/**
 * The spatial part of the 3D momentum conservation for fluid flow
 */
class INSFEFluidMomentumKernel : public INSFEFluidKernelStabilization
{
public:
  static InputParameters validParams();

  INSFEFluidMomentumKernel(const InputParameters & parameters);
  virtual ~INSFEFluidMomentumKernel() {}

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const VariableGradient & _grad_eps;
  bool _conservative_form;
  bool _p_int_by_parts;
  unsigned _component;
};
