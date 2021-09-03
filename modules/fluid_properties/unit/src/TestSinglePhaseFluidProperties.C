//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestSinglePhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", TestSinglePhaseFluidProperties);

InputParameters
TestSinglePhaseFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  return params;
}

TestSinglePhaseFluidProperties::TestSinglePhaseFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),

    _drho_dp(2.0),
    _drho_dT(3.0),
    _de_dp(4.0),
    _de_drho(5.0),
    _ds_dv(6.0),
    _ds_de(7.0),
    _dc_dv(8.0),
    _dc_de(9.0),
    _dmu_dv(10.0),
    _dmu_de(11.0),
    _dcv_dv(12.0),
    _dcv_de(13.0),
    _dcp_dv(14.0),
    _dcp_de(15.0),
    _dk_dv(16.0),
    _dk_de(17.0)
{
}

Real
TestSinglePhaseFluidProperties::rho_from_p_T(Real p, Real T) const
{
  return _drho_dp * p + _drho_dT * T;
}

void
TestSinglePhaseFluidProperties::rho_from_p_T(
    Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(p, T);
  drho_dp = _drho_dp;
  drho_dT = _drho_dT;
}

Real
TestSinglePhaseFluidProperties::e_from_p_rho(Real p, Real rho) const
{
  return _de_dp * p + _de_drho * rho;
}

void
TestSinglePhaseFluidProperties::e_from_p_rho(
    Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  e = e_from_p_rho(p, rho);
  de_dp = _de_dp;
  de_drho = _de_drho;
}

Real
TestSinglePhaseFluidProperties::s_from_v_e(Real v, Real e) const
{
  return _ds_dv * v + _ds_de * e;
}

void
TestSinglePhaseFluidProperties::s_from_v_e(
    Real v, Real e, Real & s, Real & ds_dv, Real & ds_de) const
{
  s = s_from_v_e(v, e);
  ds_dv = _ds_dv;
  ds_de = _ds_de;
}

Real
TestSinglePhaseFluidProperties::c_from_v_e(Real v, Real e) const
{
  return _dc_dv * v + _dc_de * e;
}

void
TestSinglePhaseFluidProperties::c_from_v_e(
    Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const
{
  c = c_from_v_e(v, e);
  dc_dv = _dc_dv;
  dc_de = _dc_de;
}

Real
TestSinglePhaseFluidProperties::mu_from_v_e(Real v, Real e) const
{
  return _dmu_dv * v + _dmu_de * e;
}

void
TestSinglePhaseFluidProperties::mu_from_v_e(
    Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const
{
  mu = mu_from_v_e(v, e);
  dmu_dv = _dmu_dv;
  dmu_de = _dmu_de;
}

Real
TestSinglePhaseFluidProperties::cv_from_v_e(Real v, Real e) const
{
  return _dcv_dv * v + _dcv_de * e;
}

void
TestSinglePhaseFluidProperties::cv_from_v_e(
    Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  cv = cv_from_v_e(v, e);
  dcv_dv = _dcv_dv;
  dcv_de = _dcv_de;
}

Real
TestSinglePhaseFluidProperties::cp_from_v_e(Real v, Real e) const
{
  return _dcp_dv * v + _dcp_de * e;
}

void
TestSinglePhaseFluidProperties::cp_from_v_e(
    Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  cp = cp_from_v_e(v, e);
  dcp_dv = _dcp_dv;
  dcp_de = _dcp_de;
}

Real
TestSinglePhaseFluidProperties::k_from_v_e(Real v, Real e) const
{
  return _dk_dv * v + _dk_de * e;
}

void
TestSinglePhaseFluidProperties::k_from_v_e(
    Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const
{
  k = k_from_v_e(v, e);
  dk_dv = _dk_dv;
  dk_de = _dk_de;
}
