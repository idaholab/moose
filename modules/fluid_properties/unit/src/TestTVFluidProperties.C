//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestTVFluidProperties.h"

registerMooseObject("FluidPropertiesApp", TestTVFluidProperties);

InputParameters
TestTVFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  return params;
}

// The coefficients are arbitrary; they only need to give dh/dv and dp/dv of comparable
// magnitude to the temperature terms for v ~ 1 m^3/kg
TestTVFluidProperties::TestTVFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters), _a(400.0), _b(2.0e4), _c(1500.0), _d(3.0e5)
{
}

Real
TestTVFluidProperties::p_from_T_v(Real T, Real v) const
{
  return _a * T / v + _b / (v * v);
}

void
TestTVFluidProperties::p_from_T_v(Real T, Real v, Real & p, Real & dp_dT, Real & dp_dv) const
{
  p = p_from_T_v(T, v);
  dp_dT = _a / v;
  dp_dv = -_a * T / (v * v) - 2.0 * _b / (v * v * v);
}

Real
TestTVFluidProperties::h_from_T_v(Real T, Real v) const
{
  return _c * T + _d / v;
}

void
TestTVFluidProperties::h_from_T_v(Real T, Real v, Real & h, Real & dh_dT, Real & dh_dv) const
{
  h = h_from_T_v(T, v);
  dh_dT = _c;
  dh_dv = -_d / (v * v);
}

Real
TestTVFluidProperties::e_from_T_v(Real T, Real v) const
{
  return h_from_T_v(T, v) - p_from_T_v(T, v) * v;
}

Real
TestTVFluidProperties::mu_from_v_e(Real /*v*/, Real e) const
{
  return 1e-5 + 1e-11 * e;
}

Real
TestTVFluidProperties::k_from_v_e(Real /*v*/, Real e) const
{
  return 0.02 + 1e-8 * e;
}
