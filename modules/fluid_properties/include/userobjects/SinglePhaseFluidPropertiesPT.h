/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef SINGLEPHASEFLUIDPROPERTIESPT_H
#define SINGLEPHASEFLUIDPROPERTIESPT_H

#include "FluidProperties.h"

class SinglePhaseFluidPropertiesPT;

template <>
InputParameters validParams<SinglePhaseFluidPropertiesPT>();

/**
 * Common class for single phase fluid properties using a pressure
 * and temperature formulation
 */
class SinglePhaseFluidPropertiesPT : public FluidProperties
{
public:
  SinglePhaseFluidPropertiesPT(const InputParameters & parameters);
  virtual ~SinglePhaseFluidPropertiesPT();

  /// Molar mass (kg/mol)
  virtual Real molarMass() const = 0;
  /// Density from pressure and temperature (kg/m^3)
  virtual Real rho(Real pressure, Real temperature) const = 0;
  /// Density from pressure and temperature and its derivatives wrt pressure and temperature
  virtual void
  rho_dpT(Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const = 0;
  /// Internal energy from pressure and temperature (J/kg)
  virtual Real e(Real pressure, Real temperature) const = 0;
  /// Internal energy and its derivatives wrt pressure and temperature
  virtual void
  e_dpT(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const = 0;
  /// Density and internal energy from pressure and temperature and derivatives wrt pressure and temperature
  virtual void rho_e_dpT(Real pressure,
                         Real temperature,
                         Real & rho,
                         Real & drho_dp,
                         Real & drho_dT,
                         Real & e,
                         Real & de_dp,
                         Real & de_dT) const = 0;
  /// Speed of sound (m/s)
  virtual Real c(Real pressure, Real temperature) const = 0;
  /// Isobaric specific heat capacity (J/kg/K)
  virtual Real cp(Real pressure, Real temperature) const = 0;
  /// Isochoric specific heat capacity (J/kg/K)
  virtual Real cv(Real pressure, Real temperature) const = 0;
  /// Adiabatic index - ratio of specific heats (-)
  virtual Real gamma(Real pressure, Real temperature) const;
  /// Dynamic viscosity (Pa s)
  virtual Real mu(Real density, Real temperature) const = 0;
  /// Dynamic viscosity and its derivatives wrt density and temperature
  virtual void
  mu_drhoT(Real density, Real temperature, Real & mu, Real & dmu_drho, Real & dmu_dT) const = 0;
  /// Thermal conductivity (W/m/K)
  virtual Real k(Real pressure, Real temperature) const = 0;
  /// Specific entropy (J/kg/K)
  virtual Real s(Real pressure, Real temperature) const = 0;
  /// Specific enthalpy (J/kg)
  virtual Real h(Real p, Real T) const = 0;
  /// Specific enthalpy and its derivatives
  virtual void
  h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const = 0;
  /// Thermal expansion coefficient (-)
  virtual Real beta(Real pressure, Real temperature) const = 0;
  /// Henry's law constant for dissolution in water
  virtual Real henryConstant(Real temperature) const = 0;
  /// Henry's law constant for dissolution in water and derivative wrt temperature
  virtual void henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const = 0;

protected:
  /// IAPWS formulation of Henry's law constant for dissolution in water
  virtual Real henryConstantIAPWS(Real temperature, Real A, Real B, Real C) const;
  /// IAPWS formulation of Henry's law constant for dissolution in water and derivative wrt temperature
  virtual void
  henryConstantIAPWS_dT(Real temperature, Real & Kh, Real & dKh_dT, Real A, Real B, Real C) const;

  /// Universal gas constant (J/mol/K)
  const Real _R;
  /// Conversion of temperature from Celcius to Kelvin
  const Real _T_c2k;
};

#endif /* SINGLEPHASEFLUIDPROPERTIESPT_H */
