//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThermalSolidPropertiesMaterial.h"

/**
 * Graphite thermal properties as a function of temperature appropriate for
 * graphite moderator and reflectors \cite baker. This form of graphite is
 * formed by baking carbon at high temperature, and has different properties
 * from graphite formed by chemical vapor deposition such as pyrolitic carbon.
 *
 * The thermal properties of graphite are strongly dependent on the raw materials,
 * manufacturing process, and oxidation state, so manufacturer-specific properties,
 * if available, are preferred to the use of this user object.
 *
 * At atmospheric pressure, sublimation begins at approximately 3800 K, while
 * the lowest pressure at which melting is possible is at 103 atm and 4250 K.
 * Sublimation at reactor temperatures is possible only when the pressure
 * is many orders of magnitude lower than atmospheric, and hence these properties
 * correspond only to solid graphite.
 */
class ThermalGraphiteProperties : public ThermalSolidPropertiesMaterial
{
public:
  static InputParameters validParams();

  ThermalGraphiteProperties(const InputParameters & parameters);

  /**
   * Molar mass
   * A typical chemically-purified nuclear grade graphite contains less than
   * 10 ppm Si, 10 ppm Al, 5 ppm Fe, 10 ppm Ca, 1 ppm Mg, 1 ppm Ni, 1 ppm Ti,
   * 5 ppm V, and 1 ppm B such that the chemical composition is nearly entirely
   * carbon \cite baker, so the molar mass is approximated as that for natural carbon.
   * @return molar mass (kg/mol)
   */
  virtual Real molarMass() const override;

  /**
   * Isobaric specific heat capacity (J/kg$\cdot$K) as a function of temperature (K).
   * Correlation is from \cite butland and is valid over
   * 200 K $\le$ T $\le$ 3500. All coke-based graphites show approximately the same
   * specific heat, so it may be reasonable to use a single correlation to describe
   * with reasonable accuracy a wide range of grades of nuclear graphite \cite baker.
   */
  virtual void computeIsobaricSpecificHeat() override;

  /// Isobaric specific heat capacity derivatives
  virtual void computeIsobaricSpecificHeatDerivatives() override;

  /**
   * Thermal conductivity (W/m$\cdot$K) as a function of temperature (K).
   * More so than density and specific heat, thermal conductivity is extremely
   * dependent on material and manufacturing details, and no correlation is
   * provided in the review article by Baker \cite baker.
   *
   * Due to the hexagonal lattice structure typical of
   * most graphites, the thermal conductivity is about two orders of magnitude
   * higher in the planar direction than in the perpendicular direction.
   * Measurements of thermal conductivity for three different grades of nuclear
   * graphite produced by GrafTech International Holdings Inc. (PCEA, PCIB-SFG,
   * and PPEA) in the range of 25 to 900 $\degreeC$ show a difference of between
   * 15 and 25 W/m$\cdot$K between the three grades \cite albers. Experimental data is
   * provided by McEligot et. al for isotropic G-348 graphite for
   * 25 $\degree$ C $\le$ T $\le$ 1000 $\degree$ C in the form of a parabolic
   * correlation; this data is re-fit with a
   * power correlation to permit (cautionary) extrapolation to
   * higher temperatures without giving non-physical increases in thermal
   * conductivity at the local minima of the parabolic fit \cite mceligot. The
   * R$^2$ value of this fit is 0.97620. The systematic uncertainty is $\pm$2.7% while
   * the random uncertainty is $\pm$1.5%. At low temperatures, this correlation
   * slightly underpredicts the GrafTech data \cite albers.
   */
  virtual void computeThermalConductivity() override;

  /// Thermal conductivity derivatives
  virtual void computeThermalConductivityDerivatives() override;

  /**
   * Density (kg/m$^3$) as a function of temperature (K).
   * Given a room-temperature density, the thermal expansion coefficient is used
   * to compute the temperature dependence \cite baker.
   */
  virtual void computeDensity() override;

  /// Density derivatives
  virtual void computeDensityDerivatives() override;

  /**
   * Thermal expansion coefficient (1/K) as a function of temperature (K).
   * The mean thermal expansion coefficient in the
   * range of 20 $\degree C$ to some end temperature can be approximated by adding
   * a constant to the average thermal expansion coefficient in the 20 to 100
   * $\degree C$ range \cite baker. Tabular data is fit to estimate this additive
   * constant in the range of 100 to 2500 $\degree C$. This approximates the
   * thermal expansion coefficient as a constant over possibly a wide temperature
   * range, which may give inaccurate densities at higher temperatures. However,
   * the very small variation of density with temperature indicates that despite this
   * error, the variation of density is not very significant anyways.
   */
  Real beta() const;

protected:
  /**
   * Density at room temperature, with a default value of 1600.0 kg/m$^3$ taken as the
   * midpoint of the 1.5 to 1.7 g/cm$^3$ range given by Baker \cite baker.
   */
  const Real & _rho_room_temp;

  /**
   * Average thermal expansion coefficient over
   * 20 $\degree$ C $\le$ T $\le$ 100 $\degree$ C. An average is
   * taken over the parallel and transverse thermal expansion coefficients of two
   * grades of graphite (KC and ATJ) \cite baker.
   */
  const Real _beta0;

private:
  /// The solid name
  static const std::string _name;
};
