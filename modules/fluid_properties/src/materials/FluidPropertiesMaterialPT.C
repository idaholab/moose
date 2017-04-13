/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "FluidPropertiesMaterialPT.h"

template <>
InputParameters
validParams<FluidPropertiesMaterialPT>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("pressure", "Fluid pressure (Pa)");
  params.addRequiredCoupledVar("temperature", "Fluid temperature (C)");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription(
      "Material properties for a fluid using the (pressure, temperature) formulation");
  return params;
}

FluidPropertiesMaterialPT::FluidPropertiesMaterialPT(const InputParameters & parameters)
  : Material(parameters),
    _pressure(coupledValue("pressure")),
    _temperature(coupledValue("temperature")),

    _rho(declareProperty<Real>("density")),
    _mu(declareProperty<Real>("viscosity")),
    _cp(declareProperty<Real>("cp")),
    _cv(declareProperty<Real>("cv")),
    _k(declareProperty<Real>("k")),
    _h(declareProperty<Real>("h")),
    _e(declareProperty<Real>("e")),
    _s(declareProperty<Real>("s")),
    _c(declareProperty<Real>("c")),

    _fp(getUserObject<SinglePhaseFluidPropertiesPT>("fp"))
{
}

FluidPropertiesMaterialPT::~FluidPropertiesMaterialPT() {}

void
FluidPropertiesMaterialPT::computeQpProperties()
{
  _rho[_qp] = _fp.rho(_pressure[_qp], _temperature[_qp]);
  _mu[_qp] = _fp.mu(_rho[_qp], _temperature[_qp]);
  _cp[_qp] = _fp.cp(_pressure[_qp], _temperature[_qp]);
  _cv[_qp] = _fp.cv(_pressure[_qp], _temperature[_qp]);
  _k[_qp] = _fp.k(_rho[_qp], _temperature[_qp]);
  _h[_qp] = _fp.h(_pressure[_qp], _temperature[_qp]);
  _e[_qp] = _fp.e(_pressure[_qp], _temperature[_qp]);
  _s[_qp] = _fp.s(_pressure[_qp], _temperature[_qp]);
  _c[_qp] = _fp.c(_pressure[_qp], _temperature[_qp]);
}
