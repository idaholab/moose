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
 * PowerGas form of relative permeability
 * Define s = (seff - simm)/(1 - simm).
 * Then relperm = 1 - (n+1)(1-s)^n + n(1-s)^(n+1) if s<simm, otherwise relperm=1
 */
class RichardsRelPermPowerGas : public RichardsRelPerm
{
public:
  static InputParameters validParams();

  RichardsRelPermPowerGas(const InputParameters & parameters);

  /**
   * Relative permeability
   * @param seff effective saturation
   */
  Real relperm(Real seff) const;

  /**
   * Derivative of relative permeability wrt seff
   * @param seff effective saturation
   */
  Real drelperm(Real seff) const;

  /**
   * Second derivative of relative permeability wrt seff
   * @param seff effective saturation
   */
  Real d2relperm(Real seff) const;

protected:
  /// immobile saturation
  Real _simm;

  /// exponent
  Real _n;
};
