//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADPipeWallTemperatureScalarKernelBase.h"

/**
 * Outer wall path-integrated incompressible conjugate heat transfer kernel with an applied heat
 * flux boundary condition
 */
class ADPipeOuterWallHeatFluxScalarKernel : public ADPipeWallTemperatureScalarKernelBase
{
  using Base = ADPipeWallTemperatureScalarKernelBase;

public:
  ADPipeOuterWallHeatFluxScalarKernel(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal otherNodeTemperature() const override { return _Tin[0]; }
  virtual ADReal convectiveLinearHeatRate() const override;
  virtual Real convectiveJacobian() const override;

  /// Coupled temperature of the inner-surface wall node
  const ADVariableValue & _Tin;
  /// Applied heat flux functor
  const Moose::Functor<ADReal> & _heat_flux;
  /// Perimeter of this wall node over which the heat flux is applied
  const Moose::Functor<ADReal> & _heated_perimeter;
};
