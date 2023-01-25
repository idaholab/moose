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
 * The spatial part of the 3D mass conservation for fluid flow
 */
class INSFEFluidMassKernel : public INSFEFluidKernelStabilization
{
public:
  static InputParameters validParams();

  INSFEFluidMassKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  // Additional coupled variables
  const VariableSecond & _u_vel_second;
  const VariableSecond & _v_vel_second;
  const VariableSecond & _w_vel_second;
};
