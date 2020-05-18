//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TwoPhaseFluidProperties.h"
#include "VaporMixtureFluidProperties.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * Base class for fluid properties used with 2-phase flow with non-condensable
 * gases (NCGs) present.
 */
class TwoPhaseNCGFluidProperties : public TwoPhaseFluidProperties
{
public:
  static InputParameters validParams();

  TwoPhaseNCGFluidProperties(const InputParameters & parameters);

  const UserObjectName & getLiquidName() const override { return _fp_2phase->getLiquidName(); }
  const UserObjectName & getVaporName() const override { return _fp_2phase->getVaporName(); }

  /**
   * Returns the name of the vapor mixture fluid properties object
   */
  const UserObjectName & getVaporMixtureName() const { return _vapor_mixture_name; }

  /**
   * Returns the number of non-condensable gases
   */
  unsigned int getNumberOfNCGs() const { return _fp_vapor_mixture->getNumberOfSecondaryVapors(); }

  /**
   * Returns the critical pressure
   */
  virtual Real p_critical() const override { return _fp_2phase->p_critical(); }

  /**
   * Computes the saturation temperature at a pressure
   *
   * @param[in] p  pressure
   */
  virtual Real T_sat(Real p) const override { return _fp_2phase->T_sat(p); }

  /**
   * Computes the saturation pressure at a temperature
   *
   * @param[in] T  temperature
   */
  virtual Real p_sat(Real T) const override { return _fp_2phase->p_sat(T); }

  /**
   * Computes dT/dp along the saturation line
   *
   * @param[in] p  pressure
   */
  virtual Real dT_sat_dp(Real p) const override { return _fp_2phase->dT_sat_dp(p); }

  /**
   * Computes latent heat of vaporization
   *
   * @param p  pressure
   * @param T  temperature
   */
  virtual Real h_lat(Real p, Real T) const override { return _fp_2phase->h_lat(p, T); }

  virtual bool supportsPhaseChange() const override { return _fp_2phase->supportsPhaseChange(); }

protected:
  /// Two-phase fluid properties user object name
  const UserObjectName _2phase_name;
  /// Vapor mixture fluid properties user object name
  const UserObjectName _vapor_mixture_name;

  /// Two-phase fluid properties user object
  const TwoPhaseFluidProperties * _fp_2phase;
  /// Vapor mixture fluid properties user object
  const VaporMixtureFluidProperties * _fp_vapor_mixture;
};

#pragma GCC diagnostic pop
