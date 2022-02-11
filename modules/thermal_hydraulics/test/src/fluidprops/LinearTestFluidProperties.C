//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearTestFluidProperties.h"

registerMooseObject("ThermalHydraulicsTestApp", LinearTestFluidProperties);

InputParameters
LinearTestFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  return params;
}

LinearTestFluidProperties::LinearTestFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters)
{
}

Real
LinearTestFluidProperties::rho_from_p_T(Real p, Real T) const
{
  return 2. * p + 3. * T;
}

void
LinearTestFluidProperties::rho_from_p_T(
    Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = this->rho_from_p_T(p, T);
  drho_dp = 2.;
  drho_dT = 3.;
}

Real
LinearTestFluidProperties::e_from_p_rho(Real p, Real rho) const
{
  return 3. * p + 4. * rho;
}

void
LinearTestFluidProperties::e_from_p_rho(
    Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  e = this->e_from_p_rho(p, rho);
  de_dp = 3.;
  de_drho = 4.;
}

Real
LinearTestFluidProperties::T_from_v_e(Real v, Real e) const
{
  return 2. * v + 3. * e;
}

void
LinearTestFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T = this->T_from_v_e(v, e);
  dT_dv = 2.;
  dT_de = 3.;
}

Real
LinearTestFluidProperties::p_from_v_e(Real v, Real e) const
{
  return 3.5 * v + 2.1 * e;
}

void
LinearTestFluidProperties::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
{
  p = this->p_from_v_e(v, e);
  dp_dv = 3.5;
  dp_de = 2.1;
}

Real
LinearTestFluidProperties::mu_from_v_e(Real v, Real e) const
{
  return 0.6 * v + 1.3 * e;
}

void
LinearTestFluidProperties::mu_from_v_e(
    Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const
{
  mu = this->mu_from_v_e(v, e);
  dmu_dv = 0.6;
  dmu_de = 1.3;
}
