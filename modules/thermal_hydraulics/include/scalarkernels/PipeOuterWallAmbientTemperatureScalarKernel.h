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
template <bool is_ad>
class PipeOuterWallAmbientTemperatureScalarKernelTempl
  : public PipeWallTemperatureScalarKernelBaseTempl<is_ad>
{
  using Base = PipeWallTemperatureScalarKernelBaseTempl<is_ad>;

public:
  PipeOuterWallAmbientTemperatureScalarKernelTempl(const InputParameters & parameters);
  virtual bool isADObject() const override { return is_ad; };
  static InputParameters validParams();

protected:
  virtual GenericReal<is_ad> otherNodeTemperature() const override { return _Tin[0]; }
  virtual GenericReal<is_ad> convectiveResidual() const override;
  virtual Real convectiveJacobian() const override;

  /// Coupled temperature of the inner-surface wall node
  const VariableValue & _Tin;
  /// Ambient temperature functor
  const Moose::Functor<GenericReal<is_ad>> & _T_ambient;
  /// Ambient heat transfer coefficient functor
  const Moose::Functor<GenericReal<is_ad>> & _htc_ambient;
  /// Perimeter of this wall node exposed to the ambient environment
  const Moose::Functor<GenericReal<is_ad>> & _ambient_perimeter;
};

typedef PipeOuterWallAmbientTemperatureScalarKernelTempl<false>
    PipeOuterWallAmbientTemperatureScalarKernel;
typedef PipeOuterWallAmbientTemperatureScalarKernelTempl<true>
    ADPipeOuterWallAmbientTemperatureScalarKernel;
