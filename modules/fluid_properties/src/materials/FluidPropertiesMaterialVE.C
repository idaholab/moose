//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidPropertiesMaterialVE.h"

registerMooseObject("FluidPropertiesApp", FluidPropertiesMaterialVE);

InputParameters
FluidPropertiesMaterialVE::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("e", "Fluid specific internal energy (J/kg)");
  params.addRequiredCoupledVar("v", "Fluid specific volume (m^3)");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription("formulation for s(h,p) and rho(v,e)");
  return params;
}

FluidPropertiesMaterialVE::FluidPropertiesMaterialVE(const InputParameters & parameters)
  : Material(parameters),
    _v(coupledValue("v")),
    _e(coupledValue("e")),

    _rho(declareProperty<Real>("density")),
    // _pressure(declareProperty<Real>("pressure")),
    // _temperature(declareProperty<Real>("temperature")),
    // _c(declareProperty<Real>("c")),
    // _cp(declareProperty<Real>("cp")),
    // _cv(declareProperty<Real>("cv")),
    // _mu(declareProperty<Real>("mu")),
    // _k(declareProperty<Real>("k")),
    // _g(declareProperty<Real>("g")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

FluidPropertiesMaterialVE::~FluidPropertiesMaterialVE() {}

void
FluidPropertiesMaterialVE::computeQpProperties()
{
  // _pressure[_qp] = _fp.p_from_v_e(_v[_qp], _e[_qp]);
  // _temperature[_qp] = _fp.T_from_v_e(_v[_qp], _e[_qp]);
  // _c[_qp] = _fp.c_from_v_e(_v[_qp], _e[_qp]);
  // _cp[_qp] = _fp.cp_from_v_e(_v[_qp], _e[_qp]);
  // _cv[_qp] = _fp.cv_from_v_e(_v[_qp], _e[_qp]);
  // _mu[_qp] = _fp.mu_from_v_e(_v[_qp], _e[_qp]);
  // _k[_qp] = _fp.k_from_v_e(_v[_qp], _e[_qp]);
  // _g[_qp] = _fp.g_from_v_e(_v[_qp], _e[_qp]);
  _rho[_qp] = 1 / _v[_qp];
}
