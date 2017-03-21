/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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

  /// Pressure as a function of specific internal energy and specific volume
  virtual Real pressure(Real v, Real u) const = 0;
  /// Temperature as a function of specific internal energy and specific volume
  virtual Real temperature(Real v, Real u) const = 0;
  /// Sound speed
  virtual Real c(Real v, Real u) const = 0;
  /// Specific heat
  virtual Real cp(Real v, Real u) const = 0;
  /// Isochoric specific heat
  virtual Real cv(Real v, Real u) const = 0;
  /// Compute the ratio of specific heats
  virtual Real gamma(Real v, Real u) const;
  /// Dynamic viscosity [Pa s]
  virtual Real mu(Real v, Real u) const = 0;
  /// Thermal conductivity [W / m K]
  virtual Real k(Real v, Real u) const = 0;
  /// Specific entropy [ J / kg K ]
  virtual Real s(Real v, Real u) const = 0;
  /// The derivative of pressure wrt specific volume and specific internal energy
  virtual void
  dp_duv(Real v, Real u, Real & dp_dv, Real & dp_du, Real & dT_dv, Real & dT_du) const = 0;

  /// Compute internal energy and density from specific entropy and pressure
  virtual void rho_e_ps(Real pressure, Real entropy, Real & rho, Real & e) const = 0;
  virtual void rho_e_dps(Real pressure,
                         Real entropy,
                         Real & rho,
                         Real & drho_dp,
                         Real & drho_ds,
                         Real & e,
                         Real & de_dp,
                         Real & de_ds) const = 0;

  /// Computes density from pressure and temperature
  virtual Real rho(Real pressure, Real temperature) const = 0;
  /// Computes density from pressure and temperature and its derivatives w.r.t pressure and temperature
  virtual void
  rho_dpT(Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const = 0;
  /// Computes density and internal energy from pressure and temperature
  virtual void rho_e(Real pressure, Real temperature, Real & rho, Real & e) const = 0;

  /// Computes internal energy from pressure and temperature
  virtual Real e(Real pressure, Real rho) const = 0;
  /// Computes internal energy and its derivatives of internal energy w.r.t. pressure and density
  virtual void e_dprho(Real pressure, Real rho, Real & e, Real & de_dp, Real & de_drho) const = 0;

  /// Computes specific enthalpy
  virtual Real h(Real p, Real T) const = 0;
  /// Compute enthalpy and its derivatives
  virtual void
  h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const = 0;

  /// Pressure as a function of specific enthalpy and specific entropy
  virtual Real p_from_h_s(Real h, Real s) const = 0;
  /// Derivative of pressure wrt specific enthalpy
  virtual Real dpdh_from_h_s(Real h, Real s) const = 0;

  /// Thermal expansion coefficient
  virtual Real beta(Real p, Real T) const = 0;
};

#endif /* SINGLEPHASEFLUIDPROPERTIES_H */
