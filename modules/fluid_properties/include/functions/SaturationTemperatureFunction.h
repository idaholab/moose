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

/**
 * Computes saturation temperature from pressure function and 2-phase fluid properties object
 */
class SaturationTemperatureFunction : public Function, public FunctionInterface
{
public:
  static InputParameters validParams();

  SaturationTemperatureFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;
  virtual RealVectorValue gradient(Real t, const Point & p) const override;

protected:
  /// Pressure function
  const Function & _p_fn;
  /// 2-phase fluid properties object
  const TwoPhaseFluidProperties & _fp_2phase;
};
