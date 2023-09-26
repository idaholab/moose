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
 * Computes cylindrical gap heat flux due to conduction and radiation.
 *
 * The convention is that positive heat fluxes correspond to heat moving from
 * the inner surface to the outer surface.
 */
template <bool is_ad>
class CylindricalGapHeatFluxFunctorMaterialTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  CylindricalGapHeatFluxFunctorMaterialTempl(const InputParameters & parameters);

protected:
  /// Inner surface radius
  const Moose::Functor<GenericReal<is_ad>> & _r_inner;
  /// Outer surface radius
  const Moose::Functor<GenericReal<is_ad>> & _r_outer;
  /// Inner surface temperature
  const Moose::Functor<GenericReal<is_ad>> & _T_inner;
  /// Outer surface temperature
  const Moose::Functor<GenericReal<is_ad>> & _T_outer;
  /// Gap thermal conductivity
  const Moose::Functor<GenericReal<is_ad>> & _k_gap;
  /// Inner surface emissivity
  const Moose::Functor<GenericReal<is_ad>> & _emiss_inner;
  /// Outer surface emissivity
  const Moose::Functor<GenericReal<is_ad>> & _emiss_outer;
};

typedef CylindricalGapHeatFluxFunctorMaterialTempl<false> CylindricalGapHeatFluxFunctorMaterial;
typedef CylindricalGapHeatFluxFunctorMaterialTempl<true> ADCylindricalGapHeatFluxFunctorMaterial;
