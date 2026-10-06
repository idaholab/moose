//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADScalarKernel.h"
#include "ScalarKernel.h"
#include "FunctorInterface.h"

class ThermalSolidProperties;

template <bool is_ad>
class PipeOuterWallRadiationScalarKernelTempl
  : public std::conditional<is_ad, ADScalarKernel, ScalarKernel>::type,
    public FunctorInterface
{
  using Base = typename std::conditional<is_ad, ADScalarKernel, ScalarKernel>::type;

public:
  PipeOuterWallRadiationScalarKernelTempl(const InputParameters & parameters);
  virtual bool isADObject() const override { return is_ad; };
  static InputParameters validParams();

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Solid properties object for the pipe wall
  const ThermalSolidProperties & _sp;
  /// Property state integration flag
  const bool _is_implicit;
  /// Cross-sectional area of this wall layer
  const Moose::Functor<GenericReal<is_ad>> & _area;
  /// Perimeter of this wall node exposed to the radiative environment
  const Moose::Functor<GenericReal<is_ad>> & _perimeter;
  /// Ambient temperature functor
  const Moose::Functor<GenericReal<is_ad>> & _T_ambient;
  /// Emissivity functor
  const Moose::Functor<GenericReal<is_ad>> & _emissivity;
  /// View factor functor
  const Moose::Functor<GenericReal<is_ad>> & _view_factor;
  /// Functor by which to scale the radiative heat transfer term
  const Moose::Functor<GenericReal<is_ad>> & _scale;
  /// Stefan-Boltzmann constant
  const Real _sigma;
};

typedef PipeOuterWallRadiationScalarKernelTempl<false> PipeOuterWallRadiationScalarKernel;
typedef PipeOuterWallRadiationScalarKernelTempl<true> ADPipeOuterWallRadiationScalarKernel;
