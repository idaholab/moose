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
 * Computes a heat transfer coefficient using the Churchill-Chu correlation for natural convection.
 */
template <bool is_ad>
class ChurchillChuHTCFunctorMaterialTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  ChurchillChuHTCFunctorMaterialTempl(const InputParameters & parameters);

protected:
  /// Fluid Prandtl number
  const Moose::Functor<GenericReal<is_ad>> & _Pr;
  /// Grashof number
  const Moose::Functor<GenericReal<is_ad>> & _Gr;
  /// Fluid thermal conductivity
  const Moose::Functor<GenericReal<is_ad>> & _k_fluid;
  /// Cylinder diameter
  const Real _diameter;
};

typedef ChurchillChuHTCFunctorMaterialTempl<false> ChurchillChuHTCFunctorMaterial;
typedef ChurchillChuHTCFunctorMaterialTempl<true> ADChurchillChuHTCFunctorMaterial;
