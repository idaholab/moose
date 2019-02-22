//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MULTICOMPONENTFLUIDPROPERTIESPT_H
#define MULTICOMPONENTFLUIDPROPERTIESPT_H

#include "FluidProperties.h"
#include "SinglePhaseFluidProperties.h"

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

  /**
   * Fluid name
   * @return string representing fluid name
   */
  virtual std::string fluidName() const = 0;

  /**
   * Density
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xmass mass fraction (-)
   * @return density (kg/m^3)
   */
  virtual Real rho(Real pressure, Real temperature, Real xmass) const = 0;

  /**
   * Density and its derivatives wrt pressure, temperature and mass fraction
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xmass mass fraction (-)
   * @param[out] rho density (kg/m^3)
   * @param[out] drho_dp derivative of density wrt pressure
   * @param[out] drho_dT derivative of density wrt temperature
   * @param[out] drho_dx derivative of density wrt mass fraction
   */
  virtual void rho_dpTx(Real pressure,
                        Real temperature,
                        Real xmass,
                        Real & rho,
                        Real & drho_dp,
                        Real & drho_dT,
                        Real & drho_dx) const = 0;

  /*
   * Dynamic viscosity
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xmass mass fraction (-)
   * @return viscosity (Pa.s)
   */
  virtual Real mu(Real pressure, Real temperature, Real xmass) const = 0;

  /*
   * Dynamic viscosity and its derivatives wrt pressure and temperature
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xmass mass fraction (-)
   * @param[out] mu viscosity (Pa.s)
   * @param[out] dmu_dp derivative of viscosity wrt pressure
   * @param[out] dmu_dT derivative of viscosity wrt temperature
   * @param[out] dmu_dx derivative of viscosity wrt mass fraction
   */
  virtual void mu_dpTx(Real pressure,
                       Real temperature,
                       Real xmass,
                       Real & mu,
                       Real & dmu_dp,
                       Real & dmu_dT,
                       Real & dmu_dx) const = 0;

  /**
   * Density and viscosity
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xmass mass fraction (-)
   * @param[out] rho density (kg/m^3)
   */
  virtual void rho_mu(Real pressure, Real temperature, Real xmass, Real & rho, Real & mu) const = 0;

  /**
   * Density and viscosity and their derivatives wrt pressure, temperature and mass fraction
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xmass mass fraction (-)
   * @param[out] rho density (kg/m^3)
   * @param[out] drho_dp derivative of density wrt pressure
   * @param[out] drho_dT derivative of density wrt temperature
   * @param[out] drho_dx derivative of density wrt mass fraction
   * @param[out] mu viscosity (Pa.s)
   * @param[out] dmu_dp derivative of viscosity wrt pressure
   * @param[out] dmu_dT derivative of viscosity wrt temperature
   * @param[out] dmu_dx derivative of viscosity wrt mass fraction
   */
  virtual void rho_mu_dpTx(Real pressure,
                           Real temperature,
                           Real xmass,
                           Real & rho,
                           Real & drho_dp,
                           Real & drho_dT,
                           Real & drho_dx,
                           Real & mu,
                           Real & dmu_dp,
                           Real & dmu_dT,
                           Real & dmu_dx) const = 0;

  /**
   * Enthalpy
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xmass mass fraction (-)
   * @return enthalpy (J/kg)
   */
  virtual Real h(Real pressure, Real temperature, Real xmass) const = 0;

  /**
   * Enthalpy and derivatives wrt pressure, temperature and mass fraction
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xmass mass fraction (-)
   * @param[out] h enthalpy (J/kg)
   * @param[out] dh_dp derivative of enthalpy wrt pressure
   * @param[out] dh_dT derivative of enthalpy wrt temperature
   * @param[out] dh_dx derivative of enthalpy wrt mass fraction
   */
  virtual void h_dpTx(Real pressure,
                      Real temperature,
                      Real xmass,
                      Real & h,
                      Real & dh_dp,
                      Real & dh_dT,
                      Real & dh_dx) const = 0;

  /**
   * Isobaric specific heat capacity
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xmass mass fraction (-)
   * @return cp (J/kg/K)
   */
  virtual Real cp(Real pressure, Real temperature, Real xmass) const = 0;

  /**
   * Internal energy
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xmass mass fraction (-)
   * @return internal energy (J/kg)
   */
  virtual Real e(Real pressure, Real temperature, Real xmass) const = 0;

  /**
   * Internal energy and its derivatives wrt pressure, temperature and mass fraction
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xmass mass fraction (-)
   * @param[out] e internal energy (J/kg)
   * @param[out] de_dp derivative of internal energy wrt pressure
   * @param[out] de_dT derivative of internal energy wrt temperature
   * @param[out] de_dx derivative of internal energy wrt mass fraction
   */
  virtual void e_dpTx(Real pressure,
                      Real temperature,
                      Real xmass,
                      Real & e,
                      Real & de_dp,
                      Real & de_dT,
                      Real & de_dx) const = 0;

  /**
   * Thermal conductivity
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xmass mass fraction (-)
   * @return thermal conductivity (W/m/K)
   */
  virtual Real k(Real pressure, Real temperature, Real xmass) const = 0;

  /**
   * Get UserObject for specified component
   * @param component fluid component
   * @return reference to SinglePhaseFluidPropertiesPT UserObject for component
   */
  virtual const SinglePhaseFluidProperties & getComponent(unsigned int component) const = 0;
};

#endif /* MULTICOMPONENTFLUIDPROPERTIESPT_H */
