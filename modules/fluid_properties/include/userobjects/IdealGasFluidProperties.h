//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef IDEALGASFLUIDPROPERTIES_H
#define IDEALGASFLUIDPROPERTIES_H

#include "SinglePhaseFluidProperties.h"

class IdealGasFluidProperties;

template <>
InputParameters validParams<IdealGasFluidProperties>();

/**
 * Ideal gas fluid properties
 */
class IdealGasFluidProperties : public SinglePhaseFluidProperties
{
public:
  IdealGasFluidProperties(const InputParameters & parameters);
  virtual ~IdealGasFluidProperties();

  virtual Real pressure(Real v, Real u) const override;
  virtual Real temperature(Real v, Real u) const override;
  virtual Real c(Real v, Real u) const override;
  virtual Real cp(Real v = 0., Real u = 0.) const override;
  virtual Real cv(Real v = 0., Real u = 0.) const override;
  virtual Real gamma(Real v = 0., Real u = 0.) const override;
  virtual Real mu(Real v, Real u) const override;
  virtual Real k(Real v, Real u) const override;
  virtual Real s(Real v, Real u) const override;
  virtual void s_from_h_p(Real h, Real p, Real & s, Real & ds_dh, Real & ds_dp) const override;
  virtual void
  dp_duv(Real v, Real u, Real & dp_dv, Real & dp_du, Real & dT_dv, Real & dT_du) const override;

  /// Compute internal energy and density from specific entropy and pressure
  virtual void rho_e_ps(Real pressure, Real entropy, Real & rho, Real & e) const override;
  virtual void rho_e_dps(Real pressure,
                         Real entropy,
                         Real & rho,
                         Real & drho_dp,
                         Real & drho_ds,
                         Real & e,
                         Real & de_dp,
                         Real & de_ds) const override;

  virtual Real beta(Real p, Real T) const override;

  virtual void rho_e(Real pressure, Real temperature, Real & rho, Real & e) const override;
  virtual Real rho(Real pressure, Real temperature) const override;
  virtual void rho_dpT(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;
  virtual void e_dpT(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const;

  virtual Real e(Real pressure, Real rho) const override;
  virtual void
  e_dprho(Real pressure, Real rho, Real & e, Real & de_dp, Real & de_drho) const override;

  virtual Real h(Real pressure, Real temperature) const override;
  virtual void
  h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const override;

  virtual Real p_from_h_s(Real h, Real s) const override;
  virtual Real dpdh_from_h_s(Real h, Real s) const override;
  virtual Real dpds_from_h_s(Real h, Real s) const override;

  virtual Real g(Real v, Real e) const override;

protected:
  Real _gamma;
  Real _R;
  Real _cv;
  Real _cp;

  Real _beta;
  Real _mu;
  Real _k;
};

#endif /* IDEALGASFLUIDPROPERTIES_H */
