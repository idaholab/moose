//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SODIUMPROPERTIES_H
#define SODIUMPROPERTIES_H

#include "FluidProperties.h"
#include "Moose.h"

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
   * Thermal conductivity as a function of temperature
   * @param T temperature
   * From page 181. Valid from 371 to 1500 K.
   */
  Real k(Real temperature) const;

  /**
   * Enthalpy of liquid Na (relative to solid Na at STP) in J/kg as a function of temperature
   * @param T temperature
   * From page 4. Valid from 371 to 2000 K, and relative to enthalpy of solid Na at 298.15.
   */
  Real h(Real temperature) const;

  /**
   * Heat capacity of liquid Na in J/kg-K as a function of temperature
   * @param T temperature
   * From page 29 (or by differentiating enthalpy). Valid from 371 to 2000 K.
   */
  Real heatCapacity(Real temperature) const;

  /**
   * Inverse solve for temperature [K] from enthalpy [J/kg]
   * @param h enthalpy
   */
  Real temperature(Real enthalpy) const;

  /**
   * Density as a function of temperature
   * @param T temperature
   */
  Real rho(Real temperature) const;

  /**
   * Derivative of density w.r.t temperature
   * @param T temperature
   */
  Real drho_dT(Real temperature) const;

  /**
   * Derivative of density w.r.t enthalpy
   * @param h enthalpy
   */
  Real drho_dh(Real enthalpy) const;
};

#endif // SODIUMPROPERTIES_H
