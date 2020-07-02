//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FluidProperties.h"

class SinglePhaseFluidProperties;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * Base class for fluid properties used with two-phase flow
 */
class TwoPhaseFluidProperties : public FluidProperties
{
public:
  static InputParameters validParams();

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
   * Returns the triple-point temperature
   */
  virtual Real T_triple() const;

  /**
   * Computes the saturation temperature at a pressure
   *
   * @param[in] p  pressure
   */
  virtual Real T_sat(Real p) const = 0;
  virtual DualReal T_sat(const DualReal & p) const;

  /**
   * Computes the saturation pressure at a temperature
   *
   * @param[in] T  temperature
   */
  virtual Real p_sat(Real T) const = 0;
  virtual DualReal p_sat(const DualReal & T) const;

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
  virtual DualReal h_lat(const DualReal & p, const DualReal & T) const;

  /**
   * Returns the latent heat of fusion
   */
  virtual Real L_fusion() const;

  /**
   * Computes surface tension sigma of
   * saturated liquid in contact with saturated vapor
   *
   * @param T  temperature
   */
  virtual Real sigma_from_T(Real T) const;
  virtual DualReal sigma_from_T(const DualReal & T) const;

  /**
   * Computes dsigma/dT along the saturation line
   *
   * @param[in] T          temperature (K)
   */
  virtual Real dsigma_dT_from_T(Real T) const;

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

#pragma GCC diagnostic pop
