//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ODETimeDerivative.h"
#include "ADScalarTimeDerivative.h"
#include "FunctorInterface.h"
#include "MooseTypes.h"
#include "ThermalSolidProperties.h"

class ThermalSolidProperties;

template <bool is_ad>
class PipeInnerWallTemperatureScalarKernelTempl
  : public std::conditional<is_ad, ADScalarTimeDerivative, ODETimeDerivative>::type,
    public FunctorInterface
{
  using Base = typename std::conditional<is_ad, ADScalarTimeDerivative, ODETimeDerivative>::type;

public:
  PipeInnerWallTemperatureScalarKernelTempl(const InputParameters & parameters);
  virtual bool isADObject() const override { return is_ad; };
  static InputParameters validParams();

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  const ThermalSolidProperties & _sp;
  const VariableValue & _Tout;
  const VariableValue & _Tup;
  const VariableValue & _Tdown;
  const VariableValue & _Tf;
  bool _is_implicit;
  const Moose::Functor<GenericReal<is_ad>> & _inner_diameter;
  const Moose::Functor<GenericReal<is_ad>> & _outer_diameter;
  const Moose::Functor<GenericReal<is_ad>> & _mid_diameter;
  const Moose::Functor<GenericReal<is_ad>> & _length;
  const Moose::Functor<GenericReal<is_ad>> & _upstream_spacing;
  const Moose::Functor<GenericReal<is_ad>> & _downstream_spacing;
  const Moose::Functor<GenericReal<is_ad>> & _htc;
  const Moose::Functor<GenericReal<is_ad>> & _perimeter;
};

typedef PipeInnerWallTemperatureScalarKernelTempl<false> PipeInnerWallTemperatureScalarKernel;
typedef PipeInnerWallTemperatureScalarKernelTempl<true> ADPipeInnerWallTemperatureScalarKernel;
