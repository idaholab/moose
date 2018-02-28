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
 * Base class for single phase fluid properties
 *
 * Most interfaces in this class are given as functions of specific volume
 * and specific internal energy. In some cases, specific volume and specific
 * internal energy are not the known properties, in which case it is preferred
 * practice to provide interfaces for computing specific volume and specific
 * internal energy from the known properties. This way, one can obtain all other
 * properties using the existing interfaces.
 *
 * All inputs and outputs are in SI units.
 *
 * Most interfaces have two versions: one for the value alone, and the other to
 * get the value and the derivatives with respect to the known properties.
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
  virtual Real pressure(Real v, Real e) const;

  /**
   * Temperature from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real temperature(Real v, Real e) const;

  /**
   * The derivatives of pressure and temperature w.r.t. specific volume and specific internal energy
   *
   * @param[in] v        specific volume
   * @param[in] e        specific internal energy
   * @param[out] dp_dv   derivative of pressure w.r.t. specific volume
   * @param[out] dp_de   derivative of pressure w.r.t. specific internal energy
   * @param[out] dT_dv   derivative of temperature w.r.t. specific volume
   * @param[out] dT_de   derivative of temperature w.r.t. specific internal energy
   */
  virtual void
  dp_duv(Real v, Real e, Real & dp_dv, Real & dp_de, Real & dT_dv, Real & dT_de) const;

  /**
   * Sound speed from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real c(Real v, Real e) const;

  /**
   * Sound speed and its derivatives from specific volume and specific internal energy
   *
   * @param[in]  v       specific volume
   * @param[in]  e       specific internal energy
   * @param[out] c       sound speed
   * @param[out] dc_dv   derivative of sound speed w.r.t. specific volume
   * @param[out] dc_de   derivative of sound speed w.r.t. specific internal energy
   */
  virtual void c(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const;

  /**
   * Sound speed from specific volume and specific enthalpy
   *
   * @param[in] v   specific volume
   * @param[in] h   specific enthalpy
   */
  virtual Real c_from_v_h(Real v, Real h) const;

  /**
   * Sound speed and its derivatives from specific volume and specific enthalpy
   *
   * @param[in]  v       specific volume
   * @param[in]  h       specific enthalpy
   * @param[out] c       sound speed
   * @param[out] dc_dv   derivative of sound speed w.r.t. specific volume
   * @param[out] dc_dh   derivative of sound speed w.r.t. specific enthalpy
   */
  virtual void c_from_v_h(Real v, Real h, Real & c, Real & dc_dv, Real & dc_dh) const;

  /**
   * Isobaric (constant-pressure) specific heat from specific volume and specific
   * internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real cp(Real v, Real e) const;

  /**
   * Isochoric (constant-volume) specific heat from specific volume and specific
   * internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real cv(Real v, Real e) const;

  /**
   * Ratio of specific heats from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real gamma(Real v, Real e) const;

  /**
   * Dynamic viscosity from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real mu(Real v, Real e) const;

  /**
   * Thermal conductivity from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real k(Real v, Real e) const;

  /**
   * Specific entropy from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real s(Real v, Real e) const;

  /**
   * Specific entropy and its derivatives from specific enthalpy and pressure
   *
   * @param[in] h        specific enthalpy
   * @param[in] p        pressure
   * @param[out] s       specific entropy
   * @param[out] ds_dh   derivative of specific entropy w.r.t. specific enthalpy
   * @param[out] ds_dp   derivative of specific entropy w.r.t. pressure
   */
  virtual void s_from_h_p(Real h, Real p, Real & s, Real & ds_dh, Real & ds_dp) const;

  /**
   * Density and specific internal energy from pressure and specific entropy
   *
   * @param[in] p      pressure
   * @param[in] s      specific entropy
   * @param[out] rho   density
   * @param[out] e     specific internal energy
   */
  virtual void rho_e_ps(Real p, Real s, Real & rho, Real & e) const;

  /**
   * Density and specific internal energy and their derivatives from pressure and specific entropy
   *
   * @param[in] p          pressure
   * @param[in] s          specific entropy
   * @param[out] rho       density
   * @param[out] drho_dp   derivative of density w.r.t. pressure
   * @param[out] drho_ds   derivative of density w.r.t. specific entropy
   * @param[out] e         specific internal energy
   * @param[out] de_dp     derivative of specific internal energy w.r.t. pressure
   * @param[out] de_ds     derivative of specific internal energy w.r.t. specific entropy
   */
  virtual void rho_e_dps(Real p,
                         Real s,
                         Real & rho,
                         Real & drho_dp,
                         Real & drho_ds,
                         Real & e,
                         Real & de_dp,
                         Real & de_ds) const;

  /**
   * Density from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   */
  virtual Real rho(Real p, Real T) const;

  /**
   * Density and its derivatives from pressure and temperature
   *
   * @param[in] p          pressure
   * @param[in] T          temperature
   * @param[out] rho       density
   * @param[out] drho_dp   derivative of density w.r.t. pressure
   * @param[out] drho_dT   derivative of density w.r.t. temperature
   */
  virtual void rho_dpT(Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const;

  /**
   * Density and specific internal energy from pressure and temperature
   *
   * @param[in] p      pressure
   * @param[in] T      temperature
   * @param[out] rho   density
   * @param[out] e     specific internal energy
   */
  virtual void rho_e(Real p, Real T, Real & rho, Real & e) const;

  /**
   * Specific internal energy from pressure and density
   *
   * @param[in] p     pressure
   * @param[in] rho   density
   */
  virtual Real e(Real p, Real rho) const;

  /**
   * Specific internal energy and its derivatives from pressure and density
   *
   * @param[in] p          pressure
   * @param[in] rho        density
   * @param[out] e         specific internal energy
   * @param[out] de_dp     derivative of specific internal energy w.r.t. pressure
   * @param[out] de_drho   derivative of specific internal energy w.r.t. density
   */
  virtual void e_dprho(Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const;

  /**
   * Specific enthalpy from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   */
  virtual Real h(Real p, Real T) const;

  /**
   * Specific enthalpy and its derivatives from pressure and temperature
   *
   * @param[in] p        pressure
   * @param[in] T        temperature
   * @param[out] h       specific enthalpy
   * @param[out] dh_dp   derivative of specific enthalpy w.r.t. pressure
   * @param[out] dh_dT   derivative of specific enthalpy w.r.t. temperature
   */
  virtual void h_dpT(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const;

  /**
   * Pressure from specific enthalpy and specific entropy
   *
   * @param[in] h   specific enthalpy
   * @param[in] s   specific entropy
   */
  virtual Real p_from_h_s(Real h, Real s) const;

  /**
   * Derivative of pressure wrt specific enthalpy
   *
   * @param[in] h   specific enthalpy
   * @param[in] s   specific entropy
   */
  virtual Real dpdh_from_h_s(Real h, Real s) const;

  /**
   * Derivative of pressure wrt specific entropy
   *
   * @param[in] h   specific enthalpy
   * @param[in] s   specific entropy
   */
  virtual Real dpds_from_h_s(Real h, Real s) const;

  /**
   * Gibbs free energy from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real g(Real v, Real e) const;

  /**
   * Thermal expansion coefficient from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   */
  virtual Real beta(Real p, Real T) const;
};

#endif /* SINGLEPHASEFLUIDPROPERTIES_H */
