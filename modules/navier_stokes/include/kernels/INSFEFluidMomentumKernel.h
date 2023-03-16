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

  /// Gradient of porosity
  const VariableGradient & _grad_eps;
  /// Whether conservative form to be used for the convection term
  const bool _conservative_form;
  /// Whether integration by parts to be used for the pressure gradient term
  const bool _p_int_by_parts;
  /// The component (x=0, y=1, z=2) of the momentum equation this kernel is applied
  const unsigned _component;
};
