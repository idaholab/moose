//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SIMPLEFLUIDPROPERTIES_H
#define SIMPLEFLUIDPROPERTIES_H

#include "SinglePhaseFluidPropertiesPT.h"

class SimpleFluidProperties;

template <>
InputParameters validParams<SimpleFluidProperties>();

/**
 * Fluid properties of a simple, idealised fluid
 * density=density0 * exp(P / bulk_modulus - thermal_expansion * T)
 * internal_energy = cv * T
 * enthalpy = cv * T + P / density
 * The following parameters are constant:
 * thermal expansion
 * cv
 * cp
 * bulk modulus
 * thermal conductivity
 * specific entropy
 * viscosity
 */
class SimpleFluidProperties : public SinglePhaseFluidPropertiesPT
{
public:
  SimpleFluidProperties(const InputParameters & parameters);
  virtual ~SimpleFluidProperties();

  /// Fluid name
  virtual std::string fluidName() const override;

  /// Molar mass (kg/mol)
  virtual Real molarMass() const override;

  /// Thermal expansion coefficient (1/K)
  virtual Real beta(Real pressure, Real temperature) const override;

  /// Isobaric specific heat capacity (J/kg/K)
  virtual Real cp(Real pressure, Real temperature) const override;

  /// Isochoric specific heat capacity (J/kg/K)
  virtual Real cv(Real pressure, Real temperature) const override;

  /// Speed of sound (m/s)
  virtual Real c(Real pressure, Real temperature) const override;

  /// Thermal conductivity (W/m/K)
  virtual Real k(Real pressure, Real temperature) const override;
  /// Thermal conductivity and its derivatives wrt pressure and temperature
  virtual void
  k_dpT(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const override;

  /// Thermal conductivity (W/m/K)
  virtual Real k_from_rho_T(Real density, Real temperature) const override;

  /// Specific entropy (J/kg/K)
  virtual Real s(Real pressure, Real temperature) const override;

  /// Density from pressure and temperature (kg/m^3)
  virtual Real rho(Real pressure, Real temperature) const override;

  /// Density from pressure and temperature and its derivatives wrt pressure and temperature
  virtual void rho_dpT(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;

  /// Internal energy from pressure and temperature (J/kg)
  virtual Real e(Real pressure, Real temperature) const override;

  /// Internal energy and its derivatives wrt pressure and temperature
  virtual void
  e_dpT(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const override;

  /// Density and internal energy from pressure and temperature and derivatives wrt pressure and temperature
  virtual void rho_e_dpT(Real pressure,
                         Real temperature,
                         Real & rho,
                         Real & drho_dp,
                         Real & drho_dT,
                         Real & e,
                         Real & de_dp,
                         Real & de_dT) const override;

  virtual Real mu(Real pressure, Real temperature) const override;

  virtual void
  mu_dpT(Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;

  /// Dynamic viscosity (Pa s)
  virtual Real mu_from_rho_T(Real density, Real temperature) const override;

  /// Dynamic viscosity and its derivatives wrt density and temperature
  virtual void mu_drhoT_from_rho_T(Real density,
                                   Real temperature,
                                   Real ddensity_dT,
                                   Real & mu,
                                   Real & dmu_drho,
                                   Real & dmu_dT) const override;

  /// Specific enthalpy (J/kg)
  virtual Real h(Real p, Real T) const override;

  /// Specific enthalpy and its derivatives
  virtual void
  h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const override;

  /// Henry's law constant for dissolution in water
  virtual Real henryConstant(Real temperature) const override;

  /// Henry's law constant for dissolution in water and derivative wrt temperature
  virtual void henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const override;

protected:
  /// molar mass
  const Real _molar_mass;

  /// thermal expansion coefficient
  const Real _thermal_expansion;

  /// specific heat at constant volume
  const Real _cv;

  /// specific heat at constant pressure
  const Real _cp;

  /// bulk modulus
  const Real _bulk_modulus;

  /// thermal conductivity
  const Real _thermal_conductivity;

  /// specific entropy
  const Real _specific_entropy;

  /// viscosity
  const Real _viscosity;

  /// density at zero pressure and temperature
  const Real _density0;

  /// Henry constant
  const Real _henry_constant;

  /// Porepressure coefficient: enthalpy = internal_energy + porepressure / density * _pp_coeff
  const Real _pp_coeff;
};

#endif /* SIMPLEFLUIDPROPERTIES_H */
