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

#ifdef SALINE_ENABLED
#include "default_data_store.hh"
#include "thermophysical_properties.hh"
#endif

class SalineMoltenSaltFluidProperties : public SinglePhaseFluidProperties
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
public:
  // Constructor
  static InputParameters validParams();

  SalineMoltenSaltFluidProperties(const InputParameters & parameters);

  // Unit conversion constants for communicating with Saline
  static constexpr Real kPa_to_Pa = 1.0e3;
  static constexpr Real Pa_to_kPa = 1.0 / kPa_to_Pa;
  static constexpr Real kg_to_g = 1.0e3;
  static constexpr Real g_to_kg = 1.0 / kg_to_g;
  static constexpr Real m_to_cm = 1.0e2;
  static constexpr Real cm_to_m = 1.0 / m_to_cm;
  static constexpr Real N_to_mN = 1.0e3;
  static constexpr Real mN_to_N = 1 / N_to_mN;

#ifdef SALINE_ENABLED

  /**
   * Fluid name
   *
   * @return the name of the salt
   */
  virtual std::string fluidName() const override;

  /**
   * Density from pressure and temperature
   *
   * @param[in] p   pressure (Pa)
   * @param[in] T   temperature (K)
   * @return density (kg/m$^3$)
   */
  virtual Real rho_from_p_T(Real pressure, Real temperature) const override;

  /**
   * Density and its derivatives from pressure and temperature
   *
   * @param[in] p          pressure (Pa)
   * @param[in] T          temperature (K)
   * @param[out] rho       density (kg/m$^3$)
   * @param[out] drho_dp   derivative of density w.r.t. pressure
   * @param[out] drho_dT   derivative of density w.r.t. temperature
   */
  virtual void rho_from_p_T(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;

  /**
   * Isobaric specific heat capacity from pressure and temperature
   *
   * @param p   pressure (Pa)
   * @param T   temperature (K)
   * @return isobaric specific heat (J/kg/.K)
   */
  virtual Real cp_from_p_T(Real pressure, Real temperature) const override;

  /**
   * Isobaric specific heat capacity and its derivatives from pressure and temperature
   *
   * @param[in] p       pressure (Pa)
   * @param[in] T       temperature (K)
   * @param[out] cp     isobaric specific heat (J/kg/K)
   * @param[out] dcp_dp derivative of isobaric specific heat w.r.t. pressure (J/kg/K/Pa)
   */
  virtual void cp_from_p_T(
      Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const override;

  /**
   * Dynamic viscosity from pressure and temperature
   *
   * @param p   pressure (Pa)
   * @param T   temperature (K)
   * @return dynamic viscosity (Pa.s)
   */
  virtual Real mu_from_p_T(Real pressure, Real temperature) const override;

  /**
   * Dynamic viscosity from pressure and temperature
   *
   * @param[in] p       pressure (Pa)
   * @param[in] T       temperature (K)
   * @param[out] mu     dynamic viscosity (Pa.s)
   * @param[out] dmu_dp derivative of dynamic viscosity w.r.t. pressure (s)
   * @param[out] dmu_dT derivative of dynamic viscosity w.r.t. temperature (Pa.s/K)
   */
  virtual void mu_from_p_T(
      Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;

  /**
   * Thermal conductivity from pressure and temperature
   *
   * @param p   pressure (Pa)
   * @param T   temperature (K)
   * @return thermal conductivity  (W/m.K)
   */
  virtual Real k_from_p_T(Real pressure, Real temperature) const override;

  /**
   * Thermal conductivity from pressure and temperature
   *
   * @param[in] p       pressure (Pa)
   * @param[in] T       temperature (K)
   * @param[out] k      thermal conductivity (Pa.s)
   * @param[out] dk_dp  derivative of thermal conductivity w.r.t. pressure (s)
   * @param[out] dk_dT  derivative of thermal conductivity w.r.t. temperature (Pa.s/K)
   */
  virtual void
  k_from_p_T(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const override;

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
   * Specific energy from pressure and temperature
   *
   * @param[in] p   pressure (Pa)
   * @param[in] T   temperature (K)
   * @return specific energy (J/kg)
   */
  virtual Real e_from_p_T(Real p, Real T) const override;

  /**
   * Specific energy and its derivatives from pressure and temperature
   *
   * @param[in] p        pressure (Pa)
   * @param[in] T        temperature (K)
   * @param[out] e       specific energy (J/kg)
   * @param[out] de_dp   derivative of specific energy w.r.t. pressure
   * @param[out] de_dT   derivative of specific energy w.r.t. temperature
   */
  virtual void e_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const override;

  /**
   * Temperature from pressure and specific enthalpy
   *
   * @param[in] p       pressure (Pa)
   * @param[in] h       specific enthalpy (J/kg)
   * @param[out] T      temperature (K)
   */
  virtual Real T_from_p_h(Real p, Real h) const override;

#endif

protected:
  /// The relative finite differencing step size
  const Real _fd_size;
#ifdef SALINE_ENABLED
  /// Saline DataStore object
  saline::Default_Data_Store _d;
  /// Saline interface to fluid properties
  saline::Thermophysical_Properties _tp;
  /// Name of the fluid
  std::string _fluid_name;
#endif
};

#pragma GCC diagnostic pop
