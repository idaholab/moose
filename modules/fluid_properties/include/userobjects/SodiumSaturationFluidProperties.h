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

/**
 * Fluid properties for liquid sodium at saturation conditions \cite{sas} \cite{fink}.
 */
class SodiumSaturationFluidProperties : public SinglePhaseFluidProperties
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
public:
  static InputParameters validParams();

  SodiumSaturationFluidProperties(const InputParameters & parameters);

  /**
   * Fluid name
   *
   * @return "sodium"
   */
  virtual std::string fluidName() const override;

  /**
   * Density from pressure and temperature
   *
   * @param[in] p   pressure (Pa)
   * @param[in] T   temperature (K)
   * @return density (kg/m$^3$)
   */
  virtual Real rho_from_p_T(Real p, Real T) const override;

  /**
   * Density and its derivatives from pressure and temperature
   *
   * @param[in] p          pressure (Pa)
   * @param[in] T          temperature (K)
   * @param[out] rho       density (kg/m$^3$)
   * @param[out] drho_dp   derivative of density w.r.t. pressure
   * @param[out] drho_dT   derivative of density w.r.t. temperature
   */
  virtual void
  rho_from_p_T(Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const override;
  virtual void rho_from_p_T(const DualReal & p,
                            const DualReal & T,
                            DualReal & rho,
                            DualReal & drho_dp,
                            DualReal & drho_dT) const override;

  /**
   * Specific volume from pressure and temperature
   *
   * @param[in] p   pressure (Pa)
   * @param[in] T   temperature (K)
   * @return specific volume (m$^3$/kg)
   */
  virtual Real v_from_p_T(Real p, Real T) const override;

  virtual DualReal v_from_p_T(const DualReal & p, const DualReal & T) const override;

  /**
   * Specific volume and its derivatives from pressure and temperature
   *
   * @param[in] p          pressure (Pa)
   * @param[in] T          temperature (K)
   * @param[out] v         specific volume (m$^3$/kg)
   * @param[out] dv_dp     derivative of specific volume w.r.t. pressure
   * @param[out] dv_dT     derivative of specific volume w.r.t. temperature
   */
  virtual void v_from_p_T(Real p, Real T, Real & v, Real & dv_dp, Real & dv_dT) const override;

  /**
   * Specific enthalpy from pressure and temperature
   *
   * @param[in] p   pressure (Pa)
   * @param[in] T   temperature (K)
   * @return specific enthalpy (J/kg)
   */
  virtual Real h_from_p_T(Real p, Real T) const override;

  /**
   * Specific enthalpy and its derivatives from pressure and temperature
   *
   * @param[in] p        pressure (Pa)
   * @param[in] T        temperature (K)
   * @param[out] h       specific enthalpy (J/kg)
   * @param[out] dh_dp   derivative of specific enthalpy w.r.t. pressure
   * @param[out] dh_dT   derivative of specific enthalpy w.r.t. temperature
   */
  virtual void h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const override;

  /**
   * Specific internal energy from pressure and temperature
   *
   * @param[in] p   pressure (Pa)
   * @param[in] T   temperature (K)
   * @return specific internal energy (J/kg)
   */
  virtual Real e_from_p_T(Real p, Real T) const override;

  /**
   * Specific internal energy and its derivatives from pressure and temperature
   *
   * @param[in] p        pressure (Pa)
   * @param[in] T        temperature (K)
   * @param[out] e       specific internal energy (J/kg)
   * @param[out] de_dp   derivative of specific internal energy w.r.t. pressure
   * @param[out] de_dT   derivative of specific internal energy w.r.t. temperature
   */
  virtual void e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const override;

  /**
   * Isobaric specific heat capacity from pressure and temperature
   *
   * @param p   pressure (Pa)
   * @param T   temperature (K)
   * @return isobaric specific heat (J/kg/.K)
   */
  virtual Real cp_from_p_T(Real p, Real T) const override;

  /**
   * Isobaric specific heat capacity and its derivatives from pressure and temperature
   *
   * @param[in] p       pressure (Pa)
   * @param[in] T       temperature (K)
   * @param[out] cp     isobaric specific heat (J/kg/K)
   * @param[out] dcp_dp derivative of isobaric specific heat w.r.t. pressure (J/kg/K/Pa)
   */
  virtual void cp_from_p_T(Real p, Real T, Real & cp, Real & dcp_dp, Real & dcp_dT) const override;

  using SinglePhaseFluidProperties::cv_from_p_T;

  /**
   * Isochoric specific heat capacity from pressure and temperature
   *
   * @param p   pressure (Pa)
   * @param T   temperature (K)
   * @return isochoric specific heat (J/kg.K)
   */
  virtual Real cv_from_p_T(Real p, Real T) const override;
  virtual void cv_from_p_T(Real p, Real T, Real & cv, Real & dcv_dp, Real & dcv_dT) const override;

  /**
   * Molar mass
   *
   * @return molar mass (kg/mol)
   */
  virtual Real molarMass() const override;

  /**
   * Thermal conductivity from pressure and temperature
   *
   * @param p   pressure (Pa)
   * @param T   temperature (K)
   * @return thermal conductivity  (W/m.K)
   */
  virtual Real k_from_p_T(Real p, Real T) const override;

  /**
   * Thermal conductivity and its derivatives wrt pressure and temperature
   *
   * @param p           pressure (Pa)
   * @param T           temperature (K)
   * @param[out]  k     thermal conductivity  (W/m.K)
   * @param[out]  dk_dp derivative of thermal conductivity wrt pressure
   * @param[out]  dk_dT derivative of thermal conductivity wrt temperature
   */
  virtual void k_from_p_T(Real p, Real T, Real & k, Real & dk_dp, Real & dk_dT) const override;

  /**
   * Dynamic viscosity from pressure and temperature
   *
   * @param p   pressure (Pa)
   * @param T   temperature (K)
   * @return dynamic viscosity (Pa.s)
   */
  virtual Real mu_from_p_T(Real p, Real T) const override;

  /**
   * Dynamic viscosity and its derivatives wrt pressure and temperature
   *
   * @param p             pressure (Pa)
   * @param T             temperature (K)
   * @param[out] mu       viscosity (Pa.s)
   * @param[out] dmu_dp   derivative of viscosity wrt pressure
   * @param[out] dmu_dT   derivative of viscosity wrt temperature
   */
  virtual void
  mu_from_p_T(Real p, Real T, Real & mu, Real & dmu_drho, Real & dmu_dT) const override;
};
#pragma GCC diagnostic pop
