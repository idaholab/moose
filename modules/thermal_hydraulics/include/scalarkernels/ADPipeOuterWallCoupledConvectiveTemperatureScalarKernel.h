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
#include "ADFunctorInterface.h"

/**
 * Outer wall path-integrated incompressible conjugate heat transfer kernel with a coupled secondary
 * side boundary condition
 */
class ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel
  : public ADPipeWallTemperatureScalarKernelBase,
    public ADFunctorInterface
{
  using Base = ADPipeWallTemperatureScalarKernelBase;

public:
  ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal otherNodeTemperature() const override { return _Tin[0]; }
  virtual ADReal convectiveLinearHeatRate() const override;
  virtual Real convectiveJacobian() const override;

  /// Coupled mass flow rate of the secondary-side fluid
  const ADVariableValue & _m;
  /// Coupled temperature of the inner-surface wall node
  const ADVariableValue & _Tin;
  /// Coupled secondary-side fluid temperature adjacent to this wall node
  const ADVariableValue & _Tf;
  /// Coupled secondary-side fluid temperature adjacent to the upstream wall node
  const ADVariableValue & _Tfup;
  /// Coupled secondary-side fluid temperature adjacent to the downstream wall node
  const ADVariableValue & _Tfdown;
  /// Wetted perimeter of the secondary-side flow channel adjacent to this wall node
  const Function & _wetted_perimeter;
  /// Secondary-side heat transfer coefficient
  const Moose::Functor<ADReal> & _h;
};
