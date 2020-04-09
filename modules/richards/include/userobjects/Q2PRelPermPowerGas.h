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
 * Define s = seff/(1 - simm).
 * Then relperm = 1 - (n+1)s^n + ns^(n+1) if s<simm, otherwise relperm=1 or 0
 * This is suitable for gas relative permeability as a function of water saturation in Q2P models
 */
class Q2PRelPermPowerGas : public RichardsRelPerm
{
public:
  static InputParameters validParams();

  Q2PRelPermPowerGas(const InputParameters & parameters);

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
