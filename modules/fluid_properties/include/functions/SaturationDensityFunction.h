//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"
#include "FunctionInterface.h"

class TwoPhaseFluidProperties;
class SinglePhaseFluidProperties;

/**
 * Computes saturation density from temperature function
 */
class SaturationDensityFunction : public Function, public FunctionInterface
{
public:
  static InputParameters validParams();

  SaturationDensityFunction(const InputParameters & parameters);

  // To retrieve the fluid properties
  virtual void initialSetup() override;

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

protected:
  /// Temperature function
  const Function & _T_fn;
  /// 2-phase fluid properties object
  const TwoPhaseFluidProperties * _fp_2phase;
  /// Single-phase liquid properties
  const SinglePhaseFluidProperties * _fp_liquid;
  /// Single-phase vapor properties
  const SinglePhaseFluidProperties * _fp_vapor;
  /// Set true to use liquid phase; else vapor phase
  const bool _use_liquid;
};
