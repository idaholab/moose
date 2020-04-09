//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RichardsRelPerm.h"

/**
 * Monomial form of relative permeability
 * relperm = Seff^n for 0<Seff<=1, where S = (S - simm)/(1 - simm)
 * relperm = 1 for Seff>1
 * relperm = 0 for Seff<0, except if n=0 then relperm = zero_to_the_zero
 */
class RichardsRelPermMonomial : public RichardsRelPerm
{
public:
  static InputParameters validParams();

  RichardsRelPermMonomial(const InputParameters & parameters);

  /**
   * Relative permeability
   * @param seff effective saturation
   */
  Real relperm(Real seff) const;

  /**
   * Derivative of elative permeability wrt seff
   * @param seff effective saturation
   */
  Real drelperm(Real seff) const;

  /**
   * Second derivative of elative permeability wrt seff
   * @param seff effective saturation
   */
  Real d2relperm(Real seff) const;

protected:
  /// immobile saturation
  Real _simm;

  /// exponent, must be >= 0
  Real _n;

  /// 0^0, which is used if _n=0
  Real _zero_to_the_zero;
};
