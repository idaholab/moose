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
 * Inner wall path-integrated incompressible conjugate heat transfer kernel
 */
class ADPipeInnerWallTemperatureScalarKernel : public ADPipeWallTemperatureScalarKernelBase
{
  using Base = ADPipeWallTemperatureScalarKernelBase;

public:
  ADPipeInnerWallTemperatureScalarKernel(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal otherNodeTemperature() const override { return _Tout[0]; }
  virtual ADReal convectiveLinearHeatRate() const override;
  virtual Real convectiveJacobian() const override;

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
  const Moose::Functor<ADReal> & _wetted_perimeter;
  /// Heat transfer coefficient from functor material
  const Moose::Functor<ADReal> & _h;
};
