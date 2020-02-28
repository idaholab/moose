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

class SodiumProperties;

template <>
InputParameters validParams<SodiumProperties>();

/**
 * Properties of liquid sodium from ANL/RE-95/2 report "Thermodynamic and Transport Properties of
 * Sodium Liquid and Vapor" from ANL Reactor Engineering Division.
 */
class SodiumProperties : public FluidProperties
{

public:
  SodiumProperties(const InputParameters & parameters);

  /**
   * Thermal conductivity as a function of temperature. From page 181. Valid from 371 to 1500 K.
   * @param T temperature in K
   * @return Thermal conductivity of liquid sodium in W/m/K
   */
  Real k(Real temperature) const;

  /**
   * Enthalpy of liquid Na (relative to solid Na at STP) in J/kg as a function of temperature
   * From page 4. Valid from 371 to 2000 K, and relative to enthalpy of solid Na at 298.15.
   * @param T temperature in K
   * @return enthalpy of liquid sodium in J/kg
   */
  Real h(Real temperature) const;

  /**
   * Heat capacity of liquid Na in J/kg-K as a function of temperature. From page 29 (or by
   * differentiating enthalpy). Valid from 371 to 2000 K.
   * @param T temperature in K
   * @ return Heat capacity of liquid sodium in J/kg/K
   */
  Real heatCapacity(Real temperature) const;

  /**
   * Inverse solve for temperature from enthalpy
   * @param h enthalpy in J/kg
   * @return temperature of liquid sodium in K
   */
  Real temperature(Real enthalpy) const;

  /**
   * Density as a function of temperature. Valid 371 K < T < 2503.7 K
   * @param T temperature in K
   * @return density of liquid sodium in kg/m^3
   */
  Real rho(Real temperature) const;

  /**
   * Derivative of density w.r.t temperature. Valid 371 K < T < 2503.7 K
   * @param T temperature in K
   * @return derivative of density of liquid sodium with respect to temperature
   */
  Real drho_dT(Real temperature) const;

  /**
   * Derivative of density w.r.t enthalpy. Valid 371 K < T < 2503.7 K
   * @param T enthalpy in J/kg
   * @return derivative of density of liquid sodium with respect to enthalpy
   */
  Real drho_dh(Real enthalpy) const;
};
