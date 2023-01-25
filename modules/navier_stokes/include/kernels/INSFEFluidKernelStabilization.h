//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFEFluidKernelBase.h"

/**
 * Base class for stabilization kernels.
 */
class INSFEFluidKernelStabilization : public INSFEFluidKernelBase
{
public:
  static InputParameters validParams();

  INSFEFluidKernelStabilization(const InputParameters & parameters);
  virtual ~INSFEFluidKernelStabilization() {}

protected:
  virtual void precalculateResidual();

  const VariableValue & _u_dot;
  const VariableValue & _du_dot_du;
  const MaterialProperty<Real> & _tauc;
  const MaterialProperty<Real> & _taum;
  const MaterialProperty<Real> & _taue;

  RealVectorValue _vel_elem;
};
