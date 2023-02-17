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
 * This is the material class used to compute the drag coefficients in mushy
 * (porous) regions during phase change
 */
class INSFVMushyPorousFrictionMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  INSFVMushyPorousFrictionMaterial(const InputParameters & parameters);

protected:
  /// The liquid fraction
  const Moose::Functor<ADReal> & _fl;
  /// The dynamic viscosity
  const Moose::Functor<ADReal> & _mu;
  /// The liquid density
  const Moose::Functor<ADReal> & _rho_l;
  /// Dendrite spacing
  const Moose::Functor<ADReal> & _dendrite_spacing_scaling;

  /// Main closures parameters
  ///@{
  inline static const ADReal _c = 180;
  inline static const ADReal _s = 100;
  inline static const ADReal _fs_crit = 0.27;
  inline static const ADReal _forchheimer_coef = 0.55;
  ///@}
};
