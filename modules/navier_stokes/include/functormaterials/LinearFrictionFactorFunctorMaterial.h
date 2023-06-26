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
 * Material class used to compute a friction factor of the form
 * A * f(t) + B * g(t) * |v_I| with A, B vector constants, f(t) and g(t)
 * functions of time, and |v_I| the interstitial speed
 */
class LinearFrictionFactorFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  LinearFrictionFactorFunctorMaterial(const InputParameters & parameters);

protected:
  /// name of the functor computed by this material
  MooseFunctorName _functor_name;

  ///@{ A, B, f(t), g(t)
  const RealVectorValue _A;
  const RealVectorValue _B;
  const Moose::Functor<ADReal> & _f;
  const Moose::Functor<ADReal> & _g;
  ///@}

  /// Porosity
  const Moose::Functor<ADReal> & _eps;

  ///@{
  /// Superficial velocity
  const Moose::Functor<ADReal> & _superficial_vel_x;
  const Moose::Functor<ADReal> & _superficial_vel_y;
  const Moose::Functor<ADReal> & _superficial_vel_z;
  ///@}
};
