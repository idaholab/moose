//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"
#include "RZSymmetry.h"

/**
 * Integrates a cylindrical heat structure boundary convective heat flux from an external
 * application
 */
class HeatRateExternalAppConvectionRZ : public SideIntegralPostprocessor, public RZSymmetry
{
public:
  HeatRateExternalAppConvectionRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Temperature
  const VariableValue & _T;
  /// Temperature from external application
  const VariableValue & _T_ext;
  /// Heat transfer coefficient from external application
  const VariableValue & _htc_ext;
  /// Function by which to scale the heat flux
  const Function & _scale_fn;

public:
  static InputParameters validParams();
};
