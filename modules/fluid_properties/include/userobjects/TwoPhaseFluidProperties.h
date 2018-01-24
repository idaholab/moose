/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef TWOPHASEFLUIDPROPERTIES_H
#define TWOPHASEFLUIDPROPERTIES_H

#include "FluidProperties.h"

class TwoPhaseFluidProperties;
class SinglePhaseFluidProperties;

/**
 * Base class for fluid properties used with two phase flow
 */
class TwoPhaseFluidProperties : public FluidProperties
{
public:
  TwoPhaseFluidProperties(const InputParameters & parameters);

  const UserObjectName & getLiquidName() const;
  const UserObjectName & getVaporName() const;

  /**
   * Returns the critical pressure
   */
  virtual Real p_critical() const = 0;

  /**
   * Computes the saturation temperature at a pressure
   *
   * @param[in] p  pressure
   */
  virtual Real T_sat(Real p) const = 0;

  /**
   * Computes the saturation pressure at a temperature
   *
   * @param[in] T  temperature
   */
  virtual Real p_sat(Real T) const = 0;

  /**
   * Computes dT/dp along the saturation line
   *
   * @param[in] p  pressure
   */
  virtual Real dT_sat_dp(Real p) const = 0;

  /**
   * Computes latent heat of vaporization
   *
   * @param p  pressure
   * @param T  temperature
   */
  virtual Real h_lat(Real p, Real T) const;

protected:
  /// The name of the user object that provides liquid phase fluid properties
  UserObjectName _liquid_name;
  /// The name of the user object that provides vapor phase fluid properties
  UserObjectName _vapor_name;

  /// The user object that provides liquid phase fluid properties
  const SinglePhaseFluidProperties * _fp_liquid;
  /// The user object that provides vapor phase fluid properties
  const SinglePhaseFluidProperties * _fp_vapor;
};

#endif /* TWOPHASEFLUIDPROPERTIES_H */
