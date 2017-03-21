/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STIFFENEDGASFLUIDPROPERTIES_H
#define STIFFENEDGASFLUIDPROPERTIES_H

#include "SinglePhaseFluidProperties.h"

class StiffenedGasFluidProperties;

template <>
InputParameters validParams<StiffenedGasFluidProperties>();

/**
 *
 */
class StiffenedGasFluidProperties : public SinglePhaseFluidProperties
{
public:
  StiffenedGasFluidProperties(const InputParameters & parameters);
  virtual ~StiffenedGasFluidProperties();

  virtual Real pressure(Real v, Real u) const;
  virtual Real temperature(Real v, Real u) const;
  virtual Real c(Real v, Real u) const;
  virtual Real cp(Real v, Real u) const;
  virtual Real cv(Real v, Real u) const;
  virtual Real gamma(Real v, Real u) const;
  virtual Real mu(Real v, Real u) const;
  virtual Real k(Real v, Real u) const;
  virtual Real s(Real v, Real u) const;
  virtual void dp_duv(Real v, Real u, Real & dp_dv, Real & dp_du, Real & dT_dv, Real & dT_du) const;

  /// Compute internal energy and density from specific entropy and pressure
  virtual void rho_e_ps(Real pressure, Real entropy, Real & rho, Real & e) const;
  virtual void rho_e_dps(Real pressure,
                         Real entropy,
                         Real & rho,
                         Real & drho_dp,
                         Real & drho_ds,
                         Real & e,
                         Real & de_dp,
                         Real & de_ds) const;

  virtual Real beta(Real p, Real T) const;

  virtual void rho_e(Real pressure, Real temperature, Real & rho, Real & e) const;
  virtual Real rho(Real pressure, Real temperature) const;
  virtual void
  rho_dpT(Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const;
  virtual void e_dpT(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const;

  virtual Real e(Real pressure, Real rho) const;
  virtual void e_dprho(Real pressure, Real rho, Real & e, Real & de_dp, Real & de_drho) const;

  virtual Real h(Real pressure, Real temperature) const;
  virtual void h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const;

  virtual Real p_from_h_s(Real h, Real s) const;
  virtual Real dpdh_from_h_s(Real h, Real s) const;

  virtual Real c2_from_p_rho(Real pressure, Real rho) const;

protected:
  Real _gamma;
  Real _cv;
  Real _q;
  Real _q_prime;
  Real _p_inf;
  Real _cp;

  Real _mu;
  Real _k;
};

#endif /* STIFFENEDGASFLUIDPROPERTIES_H */
