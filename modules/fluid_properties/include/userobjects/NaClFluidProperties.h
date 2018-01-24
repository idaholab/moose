//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NACLFLUIDPROPERTIES_H
#define NACLFLUIDPROPERTIES_H

#include "SinglePhaseFluidPropertiesPT.h"

class NaClFluidProperties;

template <>
InputParameters validParams<NaClFluidProperties>();

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
 */
class NaClFluidProperties : public SinglePhaseFluidPropertiesPT
{
public:
  NaClFluidProperties(const InputParameters & parameters);
  virtual ~NaClFluidProperties();

  virtual std::string fluidName() const override;

  virtual Real molarMass() const override;

  /**
   * NaCl critical pressure
   * From Anderko and Pitzer, Equation of state for pure sodium chloride, Fluid
   * Phase Equil., 79 (1992)
   * @return critical pressure (Pa)
   */
  virtual Real criticalPressure() const;

  /**
   * NaCl critical temperature
   * From Anderko and Pitzer, Equation of state for pure sodium chloride, Fluid
   * Phase Equil., 79 (1992)
   * @return critical temperature (K)
   */
  virtual Real criticalTemperature() const;

  /**
   * NaCl critical density
   * From Anderko and Pitzer, Equation of state for pure sodium chloride, Fluid
   * Phase Equil., 79 (1992)
   * @return critical density (kg/m^3)
   */
  virtual Real criticalDensity() const;

  virtual Real rho(Real pressure, Real temperature) const override;

  virtual void rho_dpT(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;

  virtual Real e(Real pressure, Real temperature) const override;

  virtual void
  e_dpT(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const override;

  virtual void rho_e_dpT(Real pressure,
                         Real temperature,
                         Real & rho,
                         Real & drho_dp,
                         Real & drho_dT,
                         Real & e,
                         Real & de_dp,
                         Real & de_dT) const override;

  virtual Real c(Real pressure, Real temperature) const override;

  virtual Real cp(Real pressure, Real temperature) const override;

  virtual Real cv(Real pressure, Real temperature) const override;

  virtual Real mu(Real pressure, Real temperature) const override;

  virtual void
  mu_dpT(Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;

  virtual Real mu_from_rho_T(Real density, Real temperature) const override;

  virtual void mu_drhoT_from_rho_T(Real density,
                                   Real temperature,
                                   Real ddensity_dT,
                                   Real & mu,
                                   Real & dmu_drho,
                                   Real & dmu_dT) const override;

  virtual Real k(Real pressure, Real temperature) const override;

  virtual void
  k_dpT(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const override;

  virtual Real k_from_rho_T(Real density, Real temperature) const override;

  virtual Real s(Real pressure, Real temperature) const override;

  virtual Real h(Real pressure, Real temperature) const override;

  virtual void
  h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const override;

  virtual Real beta(Real pressure, Real temperature) const override;

  virtual Real henryConstant(Real temperature) const override;

  virtual void henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const override;

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

#endif /* NACLFLUIDPROPERTIES_H */
