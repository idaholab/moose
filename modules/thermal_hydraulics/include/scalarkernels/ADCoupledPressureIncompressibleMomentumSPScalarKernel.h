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

class ADCoupledPressureIncompressibleMomentumSPScalarKernel : public ADIncompressibleMomentumSPBase
{
  using Base = ADIncompressibleMomentumSPBase;

public:
  ADCoupledPressureIncompressibleMomentumSPScalarKernel(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal massFlowRate() const override { return _mc[0]; };
  virtual ADReal pressureDrop() const override { return -Base::_u[0]; };
  /// Coupled mass flow rate through the flow path
  const ADVariableValue & _mc;
};
