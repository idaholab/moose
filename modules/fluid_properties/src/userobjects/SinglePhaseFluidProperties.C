//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SinglePhaseFluidProperties.h"

template <>
InputParameters
validParams<SinglePhaseFluidProperties>()
{
  InputParameters params = validParams<FluidProperties>();

  return params;
}

SinglePhaseFluidProperties::SinglePhaseFluidProperties(const InputParameters & parameters)
  : FluidProperties(parameters)
{
}

SinglePhaseFluidProperties::~SinglePhaseFluidProperties() {}

Real
SinglePhaseFluidProperties::gamma_from_v_e(Real v, Real e) const
{
  return cp_from_v_e(v, e) / cv_from_v_e(v, e);
}

Real
SinglePhaseFluidProperties::e_from_p_T(Real p, Real T) const
{
  Real rho = rho_from_p_T(p, T);
  return e_from_p_rho(p, rho);
}

void
SinglePhaseFluidProperties::e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const
{
  // From rho(p,T), compute: drho(p,T)/dp, drho(p,T)/dT
  Real rho = 0., drho_dp = 0., drho_dT = 0.;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);

  // From e(p, rho), compute: de(p,rho)/dp, de(p,rho)/drho
  Real depr_dp = 0., depr_drho = 0.;
  e_from_p_rho(p, rho, e, depr_dp, depr_drho);

  // Using partial derivative rules, we have:
  // de(p,T)/dp = de(p,rho)/dp * dp/dp + de(p,rho)/drho * drho(p,T)/dp, (dp/dp == 1)
  // de(p,T)/dT = de(p,rho)/dp * dp/dT + de(p,rho)/drho * drho(p,T)/dT, (dp/dT == 0)
  de_dp = depr_dp + depr_drho * drho_dp;
  de_dT = depr_drho * drho_dT;
}

Real
SinglePhaseFluidProperties::v_from_p_T(Real p, Real T) const
{
  Real rho = rho_from_p_T(p, T);
  return 1.0 / rho;
}

void
SinglePhaseFluidProperties::v_from_p_T(Real p, Real T, Real & v, Real & dv_dp, Real & dv_dT) const
{
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);

  v = 1.0 / rho;
  const Real dv_drho = -1.0 / (rho * rho);

  dv_dp = dv_drho * drho_dp;
  dv_dT = dv_drho * drho_dT;
}

Real
SinglePhaseFluidProperties::beta_from_p_T(Real p, Real T) const
{
  // The volumetric thermal expansion coefficient is defined as
  //   1/v dv/dT)_p
  // It is the fractional change rate of volume with respect to temperature change
  // at constant pressure. Here it is coded as
  //   - 1/rho drho/dT)_p
  // using chain rule with v = v(rho)

  Real rho, drho_dp, drho_dT;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  return -drho_dT / rho;
}
