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
#include "SinglePhaseFluidProperties.h"

class SinglePhaseFluidProperties;

template <bool is_ad>
class IncompressibleMomentumSPScalarKernelTempl
  : public std::conditional<is_ad, ADScalarTimeDerivative, ODETimeDerivative>::type,
    public FunctorInterface
{
  using Base = typename std::conditional<is_ad, ADScalarTimeDerivative, ODETimeDerivative>::type;

public:
  IncompressibleMomentumSPScalarKernelTempl(const InputParameters & parameters);
  virtual bool isADObject() const override { return is_ad; };
  static InputParameters validParams();

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  const VariableValue & _dPc;
  size_t _n_temps;
  std::vector<const VariableValue *> _T;
  bool _is_implicit;
  const Moose::Functor<GenericReal<is_ad>> & _Pref;
  const SinglePhaseFluidProperties & _fp;
  size_t _n_segments;
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _areas;
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _perimeters;
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _lengths;
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _alphas;
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _forms_losses;
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _dPps;
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _roughnesses;
  const Moose::Functor<GenericReal<is_ad>> & _gravity;
};

typedef IncompressibleMomentumSPScalarKernelTempl<false> IncompressibleMomentumSPScalarKernel;
typedef IncompressibleMomentumSPScalarKernelTempl<true> ADIncompressibleMomentumSPScalarKernel;
