//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SINGLEPHASEFLUIDPROPERTIES_H
#define SINGLEPHASEFLUIDPROPERTIES_H

#include "FluidProperties.h"

class SinglePhaseFluidProperties;

template <>
InputParameters validParams<SinglePhaseFluidProperties>();

/**
 * Common class for single phase fluid properties
 */
class SinglePhaseFluidProperties : public FluidProperties
{
public:
  SinglePhaseFluidProperties(const InputParameters & parameters);
  virtual ~SinglePhaseFluidProperties();

  /**
   * Pressure from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real p_from_v_e(Real v, Real e) const = 0;

  /**
   * Pressure and its derivatives from specific volume and specific internal energy
   *
   * @param[in] v        specific volume
   * @param[in] e        specific internal energy
   * @param[out] p       pressure
   * @param[out] dp_dv   derivative of pressure w.r.t. specific volume
   * @param[out] dp_de   derivative of pressure w.r.t. specific internal energy
   */
  virtual void p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const = 0;

  /**
   * Temperature from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   * @returns sound speed
   */
  virtual Real T_from_v_e(Real v, Real e) const = 0;

  /**
   * Temperature and its derivatives from specific volume and specific internal energy
   *
   * @param[in] v        specific volume
   * @param[in] e        specific internal energy
   * @param[out] T       temperature
   * @param[out] dT_dv   derivative of temperature w.r.t. specific volume
   * @param[out] dT_de   derivative of temperature w.r.t. specific internal energy
   */
  virtual void T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const = 0;

  /**
   * Sound speed from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real c_from_v_e(Real v, Real e) const = 0;

  /**
   * Sound speed and its derivatives from specific volume and specific internal energy
   *
   * @param[in]  v       specific volume
   * @param[in]  e       specific internal energy
   * @param[out] dc_dv   derivative of sound speed w.r.t. specific volume
   * @param[out] dc_de   derivative of sound speed w.r.t. specific internal energy
   */
  virtual void c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const = 0;

  /**
   * Isobaric (constant-pressure) specific heat from specific volume and specific
   * internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real cp_from_v_e(Real v, Real e) const = 0;

  /**
   * Isochoric (constant-volume) specific heat from specific volume and specific
   * internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real cv_from_v_e(Real v, Real e) const = 0;

  /**
   * Ratio of specific heats from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real gamma_from_v_e(Real v, Real e) const;

  /**
   * Dynamic viscosity from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real mu_from_v_e(Real v, Real e) const = 0;

  /**
   * Thermal conductivity from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real k_from_v_e(Real v, Real e) const = 0;

  /**
   * Specific entropy from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real s_from_v_e(Real v, Real e) const = 0;

  /**
   * Specific entropy and its derivatives from specific volume and specific internal energy
   *
   * @param[in] v        specific volume
   * @param[in] e        specific internal energy
   * @param[out] s       specific entropy
   * @param[out] ds_dv   derivative of specific entropy w.r.t. specific volume
   * @param[out] ds_de   derivative of specific entropy w.r.t. specific internal energy
   */
  virtual void s_from_v_e(Real v, Real e, Real & s, Real & ds_dv, Real & ds_de) const = 0;

  /**
   * Specific entropy from specific enthalpy and pressure
   *
   * @param[in] h   specific enthalpy
   * @param[in] p   pressure
   */
  virtual Real s_from_h_p(Real h, Real p) const = 0;

  /**
   * Specific entropy and its derivatives from specific enthalpy and pressure
   *
   * @param[in] h        specific enthalpy
   * @param[in] p        pressure
   * @param[out] s       specific entropy
   * @param[out] ds_dh   derivative of specific entropy w.r.t. specific enthalpy
   * @param[out] ds_dp   derivative of specific entropy w.r.t. pressure
   */
  virtual void s_from_h_p(Real h, Real p, Real & s, Real & ds_dh, Real & ds_dp) const = 0;

  /**
   * Density from pressure and specific entropy
   *
   * @param[in] p   pressure
   * @param[in] s   specific entropy
   */
  virtual Real rho_from_p_s(Real p, Real s) const = 0;

  /**
   * Density and its derivatives from pressure and specific entropy
   *
   * @param[in] p          pressure
   * @param[in] s          specific entropy
   * @param[out] rho       density
   * @param[out] drho_dp   derivative of density w.r.t. pressure
   * @param[out] drho_ds   derivative of density w.r.t. specific entropy
   */
  virtual void rho_from_p_s(Real p, Real s, Real & rho, Real & drho_dp, Real & drho_ds) const = 0;

  /**
   * Specific internal energy as a function of specific volume and specific enthalpy
   *
   * @param[in] v   specific volume
   * @param[in] h   specific enthalpy
   */
  virtual Real e_from_v_h(Real v, Real h) const = 0;

  /**
   * Specific internal energy and derivatives as a function of specific volume and specific enthalpy
   *
   * @param[in]  v       specific volume
   * @param[in]  h       specific enthalpy
   * @param[out] de_dv   derivative of specific internal energy w.r.t. specific volume
   * @param[out] de_dh   derivative of specific internal energy w.r.t. specific enthalpy
   */
  virtual void e_from_v_h(Real v, Real h, Real & e, Real & de_dv, Real & de_dh) const = 0;

  /**
   * Density from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   */
  virtual Real rho_from_p_T(Real p, Real T) const = 0;

  /**
   * Density and its derivatives from pressure and temperature
   *
   * @param[in] p          pressure
   * @param[in] T          temperature
   * @param[out] rho       density
   * @param[out] drho_dp   derivative of density w.r.t. pressure
   * @param[out] drho_dT   derivative of density w.r.t. temperature
   */
  virtual void rho_from_p_T(Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const = 0;

  /**
   * Specific volume from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   */
  virtual Real v_from_p_T(Real p, Real T) const;

  /**
   * Specific volume and its derivatives from pressure and temperature
   *
   * @param[in] p          pressure
   * @param[in] T          temperature
   * @param[out] v         specific volume
   * @param[out] dv_dp     derivative of specific volume w.r.t. pressure
   * @param[out] dv_dT     derivative of specific volume w.r.t. temperature
   */
  virtual void v_from_p_T(Real p, Real T, Real & v, Real & dv_dp, Real & dv_dT) const;

  /**
   * Specific internal energy from pressure and density
   *
   * @param[in] p     pressure
   * @param[in] rho   density
   */
  virtual Real e_from_p_rho(Real p, Real rho) const = 0;

  /**
   * Specific internal energy and its derivatives from pressure and density
   *
   * @param[in] p          pressure
   * @param[in] rho        density
   * @param[out] e         specific internal energy
   * @param[out] de_dp     derivative of specific internal energy w.r.t. pressure
   * @param[out] de_drho   derivative of specific internal energy w.r.t. density
   */
  virtual void e_from_p_rho(Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const = 0;

  /**
   * Specific enthalpy from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   */
  virtual Real h_from_p_T(Real p, Real T) const = 0;

  /**
   * Specific enthalpy and its derivatives from pressure and temperature
   *
   * @param[in] p        pressure
   * @param[in] T        temperature
   * @param[out] h       specific enthalpy
   * @param[out] dh_dp   derivative of specific enthalpy w.r.t. pressure
   * @param[out] dh_dT   derivative of specific enthalpy w.r.t. temperature
   */
  virtual void h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const = 0;

  /**
   * Internal energy from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   */
  virtual Real e_from_p_T(Real p, Real T) const;

  /**
   * Internal energy and its derivatives from pressure and temperature
   *
   * @param[in] p        pressure
   * @param[in] T        temperature
   * @param[out] e       internal energy
   * @param[out] de_dp   derivative of internal energy w.r.t. pressure
   * @param[out] de_dT   derivative of internal energy w.r.t. temperature
   */
  virtual void e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const;

  /**
   * Pressure from specific enthalpy and specific entropy
   *
   * @param[in] h   specific enthalpy
   * @param[in] s   specific entropy
   */
  virtual Real p_from_h_s(Real h, Real s) const = 0;

  /**
   * Pressure and its derivatives from specific enthalpy and specific entropy
   *
   * @param[in] h   specific enthalpy
   * @param[in] s   specific entropy
   */
  virtual void p_from_h_s(Real h, Real s, Real & p, Real & dp_dh, Real & dp_ds) const = 0;

  /**
   * Gibbs free energy from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real g_from_v_e(Real v, Real e) const = 0;

  /**
   * Thermal expansion coefficient from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   */
  virtual Real beta_from_p_T(Real p, Real T) const;
};

#endif /* SINGLEPHASEFLUIDPROPERTIES_H */
