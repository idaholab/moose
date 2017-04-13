/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef MULTICOMPONENTFLUIDPROPERTIESPT_H
#define MULTICOMPONENTFLUIDPROPERTIESPT_H

#include "FluidProperties.h"
#include "SinglePhaseFluidPropertiesPT.h"

class MultiComponentFluidPropertiesPT;

template <>
InputParameters validParams<MultiComponentFluidPropertiesPT>();

/**
 * Common class for multiple component fluid properties using a pressure
 * and temperature formulation
 */
class MultiComponentFluidPropertiesPT : public FluidProperties
{
public:
  MultiComponentFluidPropertiesPT(const InputParameters & parameters);
  virtual ~MultiComponentFluidPropertiesPT();

  /// Density (kg/m^3)
  virtual Real rho(Real pressure, Real temperature, Real xmass) const = 0;
  /// Density and its derivatives wrt pressure, temperature and mass fraction
  virtual void rho_dpTx(Real pressure,
                        Real temperature,
                        Real xmass,
                        Real & rho,
                        Real & drho_dp,
                        Real & drho_dT,
                        Real & drho_dx) const = 0;
  /// Dynamic viscosity (Pa s)
  virtual Real mu(Real density, Real temperature, Real xmass) const = 0;
  /// Dynamic viscosity and its derivatives wrt pressure, temperature and mass fraction
  virtual void mu_drhoTx(Real density,
                         Real temperature,
                         Real xmass,
                         Real & mu,
                         Real & dmu_dp,
                         Real & dmu_dT,
                         Real & dmu_dx) const = 0;
  /// Enthalpy (J/kg)
  virtual Real h(Real pressure, Real temperature, Real xmass) const = 0;
  /// Enthalpy and its derivatives wrt pressure, temperature and mass fraction
  virtual void h_dpTx(Real pressure,
                      Real temperature,
                      Real xmass,
                      Real & h,
                      Real & dh_dp,
                      Real & dh_dT,
                      Real & dh_dx) const = 0;
  /// Isobaric specific heat capacity (J/kg/K)
  virtual Real cp(Real pressure, Real temperature, Real xmass) const = 0;
  /// Internal energy (J/kg)
  virtual Real e(Real pressure, Real temperature, Real xmass) const = 0;
  /// Internal energy and its derivatives wrt pressure, temperature and mass fraction
  virtual void e_dpTx(Real pressure,
                      Real temperature,
                      Real xmass,
                      Real & e,
                      Real & de_dp,
                      Real & de_dT,
                      Real & de_dx) const = 0;
  /// Thermal conductivity (W/m/K)
  virtual Real k(Real pressure, Real temperature, Real xmass) const = 0;
  /// Get UserObject for specified component
  virtual const SinglePhaseFluidPropertiesPT & getComponent(unsigned int component) const = 0;

protected:
  /// Conversion of temperature from Celcius to Kelvin
  const Real _T_c2k;
};

#endif /* MULTICOMPONENTFLUIDPROPERTIESPT_H */
