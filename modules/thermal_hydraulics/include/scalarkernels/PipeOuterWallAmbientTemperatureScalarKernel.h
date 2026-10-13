//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PipeWallTemperatureScalarKernelBase.h"

/**
 * Outer wall path-integrated incompressible conjugate heat transfer kernel with an ambient outer
 * boundary condition
 */
class ADPipeOuterWallAmbientTemperatureScalarKernel : public ADPipeWallTemperatureScalarKernelBase
{
  using Base = ADPipeWallTemperatureScalarKernelBase;

public:
  ADPipeOuterWallAmbientTemperatureScalarKernel(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal otherNodeTemperature() const override { return _Tin[0]; }
  virtual ADReal convectiveLinearHeatRate() const override;
  virtual Real convectiveJacobian() const override;

  /// Coupled temperature of the inner-surface wall node
  const ADVariableValue & _Tin;
  /// Ambient temperature functor
  const Moose::Functor<ADReal> & _T_ambient;
  /// Ambient heat transfer coefficient functor
  const Moose::Functor<ADReal> & _htc_ambient;
  /// Perimeter of this wall node exposed to the ambient environment
  const Moose::Functor<ADReal> & _ambient_perimeter;
};
