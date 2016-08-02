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

template<>
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

  /// Density from pressure and temperature (kg/m^3)
  virtual Real rho(Real pressure, Real temperature) const = 0;
  /// Density from pressure and temperature and its derivatives wrt pressure and temperature
  virtual void rho_dpT(Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const = 0;
  /// Internal energy from pressure and temperature (kJ/kg)
  virtual Real e(Real pressure, Real temperature) const = 0;
  /// Internal energy and its derivatives wrt pressure and temperature
  virtual void e_dpT(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const = 0;
  /// Density and internal energy from pressure and temperature and derivatives wrt pressure and temperature
  virtual void rho_e_dpT(Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT, Real & e, Real & de_dp, Real & de_dT) const = 0;
  /// Speed of sound (m/s)
  virtual Real c(Real pressure, Real temperature) const = 0;
  /// Isobaric specific heat capacity (kJ/kg/K)
  virtual Real cp(Real pressure, Real temperature) const = 0;
  /// Isochoric specific heat capacity (kJ/kg/K)
  virtual Real cv(Real pressure, Real temperature) const = 0;
  /// Adiabatic index - ratio of specific heats (-)
  virtual Real gamma(Real pressure, Real temperature) const;
  /// Dynamic viscosity (Pa s)
  virtual Real mu(Real density, Real temperature) const = 0;
  /// Dynamic viscosity and its derivatives wrt density and temperature
  virtual void mu_drhoT(Real density, Real temperature, Real & mu, Real & dmu_drho, Real & dmu_dT) const = 0;
  /// Thermal conductivity (W/m/K)
  virtual Real k(Real pressure, Real temperature) const = 0;
  /// Specific entropy (kJ/kg/K)
  virtual Real s(Real pressure, Real temperature) const = 0;
  /// Specific enthalpy (kJ/kg)
  virtual Real h(Real p, Real T) const = 0;
  /// Specific enthalpy and its derivatives
  virtual void h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const = 0;
  /// Thermal expansion coefficient (-)
  virtual Real beta(Real pressure, Real temperature) const = 0;

protected:
  /// Universal gas constant (J/mol/K)
  const Real _R;
};

#endif /* SINGLEPHASEFLUIDPROPERTIESPT_H */
