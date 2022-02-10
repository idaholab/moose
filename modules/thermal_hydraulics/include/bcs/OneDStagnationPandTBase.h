//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"

class SinglePhaseFluidProperties;

/**
 * Provides common functions for stagnation pressure and temperature BC.
 */
class OneDStagnationPandTBase
{
public:
  OneDStagnationPandTBase(const SinglePhaseFluidProperties & fp);

  /**
   * Computes static density and static pressure from stagnation pressure, stagnation temperature,
   * and velocity
   *
   * @param[in] p0    stagnation pressure
   * @param[in] T0    stagnation temperature
   * @param[in] vel   velocity
   * @param[out] rho  static density
   * @param[out] p    static pressure
   */
  void rho_p_from_p0_T0_vel(Real p0, Real T0, Real vel, Real & rho, Real & p);

  /**
   * Computes derivative of static pressure w.r.t. velocity from stagnation pressure, stagnation
   * temperature, and velocity
   *
   * @param[in] p0    stagnation pressure
   * @param[in] T0    stagnation temperature
   * @param[in] vel   velocity
   *
   * @return derivative of static pressure w.r.t. velocity
   */
  Real dpdu_from_p0_T0_vel(Real p0, Real T0, Real vel);

  /**
   * Computes derivative of static density w.r.t. velocity from stagnation pressure, stagnation
   * temperature, and velocity
   *
   * @param[in] p0    stagnation pressure
   * @param[in] T0    stagnation temperature
   * @param[in] vel   velocity
   *
   * @return derivative of static density w.r.t. velocity
   */
  Real drhodu_from_p0_T0_vel(Real p0, Real T0, Real vel);

  /**
   * Computes derivative of static total energy w.r.t. velocity from stagnation pressure, stagnation
   * temperature, and velocity
   *
   * @param[in] p0    stagnation pressure
   * @param[in] T0    stagnation temperature
   * @param[in] vel   velocity
   *
   * @return derivative of static total energy w.r.t. velocity
   */
  Real dEdu_from_p0_T0_vel(Real p0, Real T0, Real vel);

protected:
  const SinglePhaseFluidProperties & _fp;
};
