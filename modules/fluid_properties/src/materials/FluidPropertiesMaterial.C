//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidPropertiesMaterial.h"
#include "SinglePhaseFluidProperties.h"

template <>
InputParameters
validParams<FluidPropertiesMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("e", "Specific internal energy");
  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription("Computes fluid properties using (u, v) formulation");
  return params;
}

FluidPropertiesMaterial::FluidPropertiesMaterial(const InputParameters & parameters)
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
    _g(declareProperty<Real>("g")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

FluidPropertiesMaterial::~FluidPropertiesMaterial() {}

void
FluidPropertiesMaterial::computeQpProperties()
{
  _p[_qp] = _fp.pressure(_v[_qp], _e[_qp]);
  _T[_qp] = _fp.temperature(_v[_qp], _e[_qp]);
  _c[_qp] = _fp.c(_v[_qp], _e[_qp]);
  _cp[_qp] = _fp.cp(_v[_qp], _e[_qp]);
  _cv[_qp] = _fp.cv(_v[_qp], _e[_qp]);
  _mu[_qp] = _fp.mu(_v[_qp], _e[_qp]);
  _k[_qp] = _fp.k(_v[_qp], _e[_qp]);
  _g[_qp] = _fp.g(_v[_qp], _e[_qp]);
}
