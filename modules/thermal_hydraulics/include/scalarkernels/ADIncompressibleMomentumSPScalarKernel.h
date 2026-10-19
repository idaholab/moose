//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIncompressibleMomentumSPBase.h"

class ADIncompressibleMomentumSPScalarKernel : public ADIncompressibleMomentumSPBase
{
  using Base = ADIncompressibleMomentumSPBase;

public:
  ADIncompressibleMomentumSPScalarKernel(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal massFlowRate() const override { return Base::_u[0]; };
  virtual ADReal pressureDrop() const override { return _dPc[0]; };
  /// Coupled reference pressure drop from inlet to outlet of the path
  const ADVariableValue & _dPc;
};
