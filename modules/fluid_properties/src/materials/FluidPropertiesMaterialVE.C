//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidPropertiesMaterialVE.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", FluidPropertiesMaterialVE);
registerMooseObjectRenamed("FluidPropertiesApp",
                           FluidPropertiesMaterial,
                           "01/01/2023 00:00",
                           FluidPropertiesMaterialVE);

InputParameters
FluidPropertiesMaterialVE::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("e", "Specific internal energy");
  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription(
      "Computes fluid properties using (specific internal energy, specific volume) formulation");
  return params;
}

FluidPropertiesMaterialVE::FluidPropertiesMaterialVE(const InputParameters & parameters)
  : Material(parameters),
    _e(coupledValue("e")),
    _v(coupledValue("v")),

    _p(declareProperty<Real>("pressure")),
    _T(declareProperty<Real>("temperature")),
    _c(declareProperty<Real>("c")),
    _cp(declareProperty<Real>("cp")),
    _cv(declareProperty<Real>("cv")),
    _mu(declareProperty<Real>("mu")),
    _k(declareProperty<Real>("k")),
    _s(declareProperty<Real>("s")),
    _g(declareProperty<Real>("g")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

FluidPropertiesMaterialVE::~FluidPropertiesMaterialVE() {}

void
FluidPropertiesMaterialVE::computeQpProperties()
{
  _p[_qp] = _fp.p_from_v_e(_v[_qp], _e[_qp]);
  _T[_qp] = _fp.T_from_v_e(_v[_qp], _e[_qp]);
  _c[_qp] = _fp.c_from_v_e(_v[_qp], _e[_qp]);
  _cp[_qp] = _fp.cp_from_v_e(_v[_qp], _e[_qp]);
  _cv[_qp] = _fp.cv_from_v_e(_v[_qp], _e[_qp]);
  _mu[_qp] = _fp.mu_from_v_e(_v[_qp], _e[_qp]);
  _k[_qp] = _fp.k_from_v_e(_v[_qp], _e[_qp]);
  _s[_qp] = _fp.s_from_v_e(_v[_qp], _e[_qp]);
  _g[_qp] = _fp.g_from_v_e(_v[_qp], _e[_qp]);
}
