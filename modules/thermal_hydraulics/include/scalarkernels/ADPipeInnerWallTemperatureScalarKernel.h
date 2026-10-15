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
#include "DittusBoelterHTCInterface.h"

/**
 * Inner wall path-integrated incompressible conjugate heat transfer kernel
 */
class ADPipeInnerWallTemperatureScalarKernel : public ADPipeWallTemperatureScalarKernelBase,
                                               public DittusBoelterHTCInterface
{
  using Base = ADPipeWallTemperatureScalarKernelBase;

public:
  ADPipeInnerWallTemperatureScalarKernel(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal otherNodeTemperature() const override { return _Tout[0]; }
  virtual ADReal convectiveLinearHeatRate() const override;
  virtual Real convectiveJacobian() const override;

  virtual const ADReal & massFlowRate() const override { return _m[0]; }
  virtual const ADReal & fluidTemperature() const override { return _Tf[0]; }
  virtual const ADReal & inletFluidTemperature() const override { return _Tfup[0]; }
  virtual const ADReal & outletFluidTemperature() const override { return _Tfdown[0]; }
  virtual Real wettedPerimeter(Real t) const override;

  /// Coupled mass flow rate through the pipe
  const ADVariableValue & _m;
  /// Coupled temperature of the outer-surface wall node
  const ADVariableValue & _Tout;
  /// Coupled fluid temperature adjacent to this wall node
  const ADVariableValue & _Tf;
  /// Coupled fluid temperature adjacent to the upstream wall node
  const ADVariableValue & _Tfup;
  /// Coupled fluid temperature adjacent to the downstream wall node
  const ADVariableValue & _Tfdown;
  /// Wetted perimeter of the fluid flow channel adjacent to this wall node
  const Function & _wetted_perimeter;
};
