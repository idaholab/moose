//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * Computes wall heat transfer coefficient using Dittus-Boelter equation
 */
template <bool is_ad>
class DittusBoelterFunctorMaterialTempl : public FunctorMaterial
{
public:
  DittusBoelterFunctorMaterialTempl(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  /// Hydraulic diameter
  const Moose::Functor<GenericReal<is_ad>> & _D_h;
  /// Thermal conductivity
  const Moose::Functor<GenericReal<is_ad>> & _k;
  /// Fluid temperature
  const Moose::Functor<GenericReal<is_ad>> & _T;
  /// Wall temperature
  const Moose::Functor<GenericReal<is_ad>> & _T_wall;
  /// The Prandtl number
  const Moose::Functor<GenericReal<is_ad>> & _prandtl;
  /// The Reynolds number
  const Moose::Functor<GenericReal<is_ad>> & _reynolds;
};

typedef DittusBoelterFunctorMaterialTempl<false> DittusBoelterFunctorMaterial;
typedef DittusBoelterFunctorMaterialTempl<true> ADDittusBoelterFunctorMaterial;
