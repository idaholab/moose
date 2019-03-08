//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TWOPHASEFLUIDPROPERTIES_H
#define TWOPHASEFLUIDPROPERTIES_H

#include "FluidProperties.h"

class TwoPhaseFluidProperties;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<TwoPhaseFluidProperties>();

/**
 * Base class for fluid properties used with two-phase flow
 */
class TwoPhaseFluidProperties : public FluidProperties
{
public:
  TwoPhaseFluidProperties(const InputParameters & parameters);

  /**
   * Returns the name of the liquid single-phase fluid properties object
   */
  virtual const UserObjectName & getLiquidName() const { return _liquid_name; }

  /**
   * Returns the name of the vapor single-phase fluid properties object
   */
  virtual const UserObjectName & getVaporName() const { return _vapor_name; }

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

  /**
   * Returns true if phase change is supported, otherwise false
   */
  virtual bool supportsPhaseChange() const = 0;

protected:
  /// The name of the user object that provides liquid phase fluid properties
  const UserObjectName _liquid_name;
  /// The name of the user object that provides vapor phase fluid properties
  const UserObjectName _vapor_name;

  /// The user object that provides liquid phase fluid properties
  const SinglePhaseFluidProperties * _fp_liquid;
  /// The user object that provides vapor phase fluid properties
  const SinglePhaseFluidProperties * _fp_vapor;
};

#endif /* TWOPHASEFLUIDPROPERTIES_H */
