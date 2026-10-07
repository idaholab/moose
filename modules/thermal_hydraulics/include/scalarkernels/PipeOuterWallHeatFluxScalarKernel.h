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

template <bool is_ad>
class PipeOuterWallHeatFluxScalarKernelTempl
  : public PipeWallTemperatureScalarKernelBaseTempl<is_ad>
{
  using Base = PipeWallTemperatureScalarKernelBaseTempl<is_ad>;

public:
  PipeOuterWallHeatFluxScalarKernelTempl(const InputParameters & parameters);
  virtual bool isADObject() const override { return is_ad; };
  static InputParameters validParams();

protected:
  virtual GenericReal<is_ad> otherNodeTemperature() const override { return _Tin[0]; }
  virtual GenericReal<is_ad> convectiveResidual() const override;
  virtual Real convectiveJacobian() const override;

  /// Coupled temperature of the inner-surface wall node
  const VariableValue & _Tin;
  /// Applied heat flux functor
  const Moose::Functor<GenericReal<is_ad>> & _heat_flux;
  /// Perimeter of this wall node over which the heat flux is applied
  const Moose::Functor<GenericReal<is_ad>> & _heated_perimeter;
};

typedef PipeOuterWallHeatFluxScalarKernelTempl<false> PipeOuterWallHeatFluxScalarKernel;
typedef PipeOuterWallHeatFluxScalarKernelTempl<true> ADPipeOuterWallHeatFluxScalarKernel;
