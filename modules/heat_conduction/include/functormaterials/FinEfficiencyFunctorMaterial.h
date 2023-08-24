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
 * Computes fin efficiency.
 */
template <bool is_ad>
class FinEfficiencyFunctorMaterialTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  FinEfficiencyFunctorMaterialTempl(const InputParameters & parameters);

protected:
  /// Heat transfer coefficient
  const Moose::Functor<GenericReal<is_ad>> & _htc;
  /// Thermal conductivity
  const Moose::Functor<GenericReal<is_ad>> & _k;
  /// Fin height
  const Moose::Functor<GenericReal<is_ad>> & _L;
  /// Ratio of the fin perimeter to its cross-sectional area
  const Moose::Functor<GenericReal<is_ad>> & _P_over_Ac;
};

typedef FinEfficiencyFunctorMaterialTempl<false> FinEfficiencyFunctorMaterial;
typedef FinEfficiencyFunctorMaterialTempl<true> ADFinEfficiencyFunctorMaterial;
