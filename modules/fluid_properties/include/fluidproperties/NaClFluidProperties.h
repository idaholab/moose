//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SinglePhaseFluidProperties.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * NaCl fluid properties as a function of pressure (Pa) and temperature (K).
 * Note: only solid state (halite) properties are currently implemented to
 * use in brine formulation
 *
 * Most properties from:
 * Driesner, The system H2O-NaCl. Part II: Correlations for molar volume,
 * enthalpy, and isobaric heat capacity from 0 to 1000 C, 1 to 500 bar, and 0
 * to 1 Xnacl, Geochimica et Cosmochimica Acta 71, 4902-4919 (2007)
 *
 * Thermal conductivity from:
 * Urqhart and Bauer, Experimental determination of single-crystal halite
 * thermal conductivity, diffusivity and specific heat from -75 C to 300 C,
 * Int. J. Rock Mech. and Mining Sci., 78 (2015)
 * Note: The function given in this reference doesn't satisfactorily match their
 * experimental data, so the data was refitted using a third order polynomial
 *
 * NaCl critical properties from:
 * From Anderko and Pitzer, Equation of state for pure sodium chloride, Fluid
 * Phase Equil., 79 (1992)
 */
class NaClFluidProperties : public SinglePhaseFluidProperties
{
public:
  static InputParameters validParams();

  NaClFluidProperties(const InputParameters & parameters);
  virtual ~NaClFluidProperties();

  virtual std::string fluidName() const override;

  virtual Real molarMass() const override;

  virtual Real criticalPressure() const override;

  virtual Real criticalTemperature() const override;

  virtual Real criticalDensity() const override;

  virtual Real triplePointPressure() const override;

  virtual Real triplePointTemperature() const override;

  virtual Real rho_from_p_T(Real pressure, Real temperature) const override;

  virtual void rho_from_p_T(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;

  virtual Real e_from_p_T(Real pressure, Real temperature) const override;

  virtual void
  e_from_p_T(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const override;

  virtual Real cp_from_p_T(Real pressure, Real temperature) const override;

  virtual void cp_from_p_T(
      Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const override;

  virtual Real cv_from_p_T(Real pressure, Real temperature) const override;

  virtual Real k_from_p_T(Real pressure, Real temperature) const override;

  virtual void
  k_from_p_T(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const override;

  virtual Real h_from_p_T(Real pressure, Real temperature) const override;

  virtual void
  h_from_p_T(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const override;

protected:
  /// NaCl molar mass (kg/mol)
  const Real _Mnacl;
  /// Critical pressure (Pa)
  const Real _p_critical;
  /// Critical temperature (K)
  const Real _T_critical;
  /// Critical density (kg/m^3)
  const Real _rho_critical;
  /// Triple point pressure (Pa)
  const Real _p_triple;
  /// Triple point temperature (K)
  const Real _T_triple;
};

#pragma GCC diagnostic pop
