//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * Computes a heat transfer enhancement factor for fins.
 */
template <bool is_ad>
class FinEnhancementFactorFunctorMaterialTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  FinEnhancementFactorFunctorMaterialTempl(const InputParameters & parameters);

protected:
  /// Fin efficiency
  const Moose::Functor<GenericReal<is_ad>> & _fin_efficiency;
  /// Fraction of the total surface area corresponding to fins
  const Moose::Functor<GenericReal<is_ad>> & _fin_area_fraction;
  /// Ratio of the total surface area with fins to the base surface area
  const Moose::Functor<GenericReal<is_ad>> & _area_increase_factor;
};

typedef FinEnhancementFactorFunctorMaterialTempl<false> FinEnhancementFactorFunctorMaterial;
typedef FinEnhancementFactorFunctorMaterialTempl<true> ADFinEnhancementFactorFunctorMaterial;
