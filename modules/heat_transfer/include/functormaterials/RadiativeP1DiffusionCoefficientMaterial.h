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
 * Computes a convection heat flux from a solid surface to a fluid.
 */
template <bool is_ad>
class RadiativeP1DiffusionCoefficientMaterialTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  RadiativeP1DiffusionCoefficientMaterialTempl(const InputParameters & parameters);

protected:
  /// Opacity
  const Moose::Functor<GenericReal<is_ad>> & _opacity;
  /// Effective Scattering Cross Section
  const Moose::Functor<GenericReal<is_ad>> & _sigma_scat_eff;
};

typedef RadiativeP1DiffusionCoefficientMaterialTempl<false> RadiativeP1DiffusionCoefficientMaterial;
typedef RadiativeP1DiffusionCoefficientMaterialTempl<true>
    ADRadiativeP1DiffusionCoefficientMaterial;
