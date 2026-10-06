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
class PipeOuterWallCoupledConvectiveTemperatureScalarKernelTempl
  : public PipeWallTemperatureScalarKernelBaseTempl<is_ad>
{
  using Base = PipeWallTemperatureScalarKernelBaseTempl<is_ad>;

public:
  PipeOuterWallCoupledConvectiveTemperatureScalarKernelTempl(const InputParameters & parameters);
  virtual bool isADObject() const override { return is_ad; };
  static InputParameters validParams();

protected:
  virtual GenericReal<is_ad> otherNodeTemperature() const override { return _Tin[0]; }
  virtual GenericReal<is_ad> convectiveResidual() const override;
  virtual Real convectiveJacobian() const override;

  /// Fluid properties object for the secondary-side fluid
  const SinglePhaseFluidProperties & _fp;
  /// Coupled mass flow rate of the secondary-side fluid
  const VariableValue & _m;
  /// Coupled temperature of the inner-surface wall node
  const VariableValue & _Tin;
  /// Coupled secondary-side fluid temperature adjacent to this wall node
  const VariableValue & _Tf;
  /// Coupled secondary-side fluid temperature adjacent to the upstream wall node
  const VariableValue & _Tfup;
  /// Coupled secondary-side fluid temperature adjacent to the downstream wall node
  const VariableValue & _Tfdown;
  /// Secondary-side system reference pressure
  const Moose::Functor<GenericReal<is_ad>> & _Pref;
  /// Cross-sectional area of the secondary-side flow channel adjacent to this wall node
  const Moose::Functor<GenericReal<is_ad>> & _flow_area;
  /// Wetted perimeter of the secondary-side flow channel adjacent to this wall node
  const Moose::Functor<GenericReal<is_ad>> & _wetted_perimeter;
};

typedef PipeOuterWallCoupledConvectiveTemperatureScalarKernelTempl<false>
    PipeOuterWallCoupledConvectiveTemperatureScalarKernel;
typedef PipeOuterWallCoupledConvectiveTemperatureScalarKernelTempl<true>
    ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel;
