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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

class TestSinglePhaseFluidProperties : public SinglePhaseFluidProperties
{
public:
  static InputParameters validParams();

  TestSinglePhaseFluidProperties(const InputParameters & parameters);

  virtual Real rho_from_p_T(Real p, Real T) const override;
  virtual void
  rho_from_p_T(Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const override;

  virtual Real e_from_p_rho(Real p, Real rho) const override;
  virtual void
  e_from_p_rho(Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const override;

  virtual Real s_from_v_e(Real v, Real e) const override;
  virtual void
  s_from_v_e(Real v, Real e, Real & s, Real & ds_dv, Real & ds_de) const override;

  virtual Real c_from_v_e(Real v, Real e) const override;
  virtual void
  c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const override;

  virtual Real mu_from_v_e(Real v, Real e) const override;
  virtual void
  mu_from_v_e(Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const override;

  virtual Real cv_from_v_e(Real v, Real e) const override;
  virtual void
  cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const override;

  virtual Real cp_from_v_e(Real v, Real e) const override;
  virtual void
  cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const override;

  virtual Real k_from_v_e(Real v, Real e) const override;
  virtual void
  k_from_v_e(Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const override;

protected:
  const Real _drho_dp;
  const Real _drho_dT;
  const Real _de_dp;
  const Real _de_drho;
  const Real _ds_dv;
  const Real _ds_de;
  const Real _dc_dv;
  const Real _dc_de;
  const Real _dmu_dv;
  const Real _dmu_de;
  const Real _dcv_dv;
  const Real _dcv_de;
  const Real _dcp_dv;
  const Real _dcp_de;
  const Real _dk_dv;
  const Real _dk_de;
};

#pragma GCC diagnostic pop
