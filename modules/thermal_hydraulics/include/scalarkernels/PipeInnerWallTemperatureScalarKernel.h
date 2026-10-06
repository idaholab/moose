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

class SinglePhaseFluidProperties;

template <bool is_ad>
class PipeInnerWallTemperatureScalarKernelTempl
  : public PipeWallTemperatureScalarKernelBaseTempl<is_ad>
{
  using Base = PipeWallTemperatureScalarKernelBaseTempl<is_ad>;

public:
  PipeInnerWallTemperatureScalarKernelTempl(const InputParameters & parameters);
  virtual bool isADObject() const override { return is_ad; };
  static InputParameters validParams();

protected:
  virtual GenericReal<is_ad> otherNodeTemperature() const override { return _Tout[0]; }
  virtual GenericReal<is_ad> convectiveResidual() const override;
  virtual Real convectiveJacobian() const override;

  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;
  /// Coupled mass flow rate through the pipe
  const VariableValue & _m;
  /// Coupled temperature of the outer-surface wall node
  const VariableValue & _Tout;
  /// Coupled fluid temperature adjacent to this wall node
  const VariableValue & _Tf;
  /// Coupled fluid temperature adjacent to the upstream wall node
  const VariableValue & _Tfup;
  /// Coupled fluid temperature adjacent to the downstream wall node
  const VariableValue & _Tfdown;
  /// System reference pressure
  const Moose::Functor<GenericReal<is_ad>> & _Pref;
  /// Cross-sectional area of the fluid flow channel adjacent to this wall node
  const Moose::Functor<GenericReal<is_ad>> & _flow_area;
  /// Wetted perimeter of the fluid flow channel adjacent to this wall node
  const Moose::Functor<GenericReal<is_ad>> & _wetted_perimeter;
};

typedef PipeInnerWallTemperatureScalarKernelTempl<false> PipeInnerWallTemperatureScalarKernel;
typedef PipeInnerWallTemperatureScalarKernelTempl<true> ADPipeInnerWallTemperatureScalarKernel;
