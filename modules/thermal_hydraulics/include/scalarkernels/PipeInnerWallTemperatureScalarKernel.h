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
#include "SinglePhaseFluidProperties.h"

class ThermalSolidProperties;
class SinglePhaseFluidProperties;

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
  virtual Real computeQpJacobian();
  const ThermalSolidProperties & _sp;
  const SinglePhaseFluidProperties & _fp;
  const VariableValue & _m;
  const VariableValue & _Tout;
  const VariableValue & _Tup;
  const VariableValue & _Tdown;
  const VariableValue & _Tf;
  const VariableValue & _Tfup;
  const VariableValue & _Tfdown;
  bool _is_implicit;
  const Moose::Functor<GenericReal<is_ad>> & _Pref;
  const Moose::Functor<GenericReal<is_ad>> & _inner_diameter;
  const Moose::Functor<GenericReal<is_ad>> & _outer_diameter;
  const Moose::Functor<GenericReal<is_ad>> & _length;
  const Moose::Functor<GenericReal<is_ad>> & _upstream_spacing;
  const Moose::Functor<GenericReal<is_ad>> & _downstream_spacing;
  const Moose::Functor<GenericReal<is_ad>> & _upstream_inner_diameter;
  const Moose::Functor<GenericReal<is_ad>> & _upstream_outer_diameter;
  const Moose::Functor<GenericReal<is_ad>> & _downstream_inner_diameter;
  const Moose::Functor<GenericReal<is_ad>> & _downstream_outer_diameter;
};

typedef PipeInnerWallTemperatureScalarKernelTempl<false> PipeInnerWallTemperatureScalarKernel;
typedef PipeInnerWallTemperatureScalarKernelTempl<true> ADPipeInnerWallTemperatureScalarKernel;
