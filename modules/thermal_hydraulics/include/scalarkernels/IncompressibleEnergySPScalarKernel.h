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

class SinglePhaseFluidProperties;

template <bool is_ad>
class IncompressibleEnergySPScalarKernelTempl
  : public std::conditional<is_ad, ADScalarTimeDerivative, ODETimeDerivative>::type,
    public FunctorInterface
{
  using Base = typename std::conditional<is_ad, ADScalarTimeDerivative, ODETimeDerivative>::type;

public:
  IncompressibleEnergySPScalarKernelTempl(const InputParameters & parameters);
  virtual bool isADObject() const override { return is_ad; };
  static InputParameters validParams();

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;
  /// Coupled mass flow rate through the segment
  const VariableValue & _m;
  /// Coupled temperature of the upstream segment
  const VariableValue & _Tup;
  /// Coupled temperature of the downstream segment
  const VariableValue & _Tdown;
  /// Coupled temperature of the component wall
  const VariableValue & _Tw;
  /// Property state integration flag
  const bool _is_implicit;
  /// System reference pressure
  const Moose::Functor<GenericReal<is_ad>> & _Pref;
  /// Flow area of the segment
  const Moose::Functor<GenericReal<is_ad>> & _area;
  /// Wetted perimeter of the segment
  const Moose::Functor<GenericReal<is_ad>> & _perimeter;
  /// Length of the segment
  const Moose::Functor<GenericReal<is_ad>> & _length;
};

typedef IncompressibleEnergySPScalarKernelTempl<false> IncompressibleEnergySPScalarKernel;
typedef IncompressibleEnergySPScalarKernelTempl<true> ADIncompressibleEnergySPScalarKernel;
