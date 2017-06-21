/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MultiComponentFluidPropertiesMaterialPT.h"

template <>
InputParameters
validParams<MultiComponentFluidPropertiesMaterialPT>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("pressure", "pressure (Pa)");
  params.addRequiredCoupledVar("temperature", "temperature (K)");
  params.addRequiredCoupledVar("xmass", "Solute mass fraction (-)");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addParam<bool>("vapor", false, "Flag to calculate vapor pressure");
  params.addClassDescription(
      "Fluid properties of a multicomponent fluid using the (pressure, temperature) formulation");
  return params;
}

MultiComponentFluidPropertiesMaterialPT::MultiComponentFluidPropertiesMaterialPT(
    const InputParameters & parameters)
  : Material(parameters),
    _pressure(coupledValue("pressure")),
    _temperature(coupledValue("temperature")),
    _xmass(coupledValue("xmass")),

    _rho(declareProperty<Real>("density")),
    _h(declareProperty<Real>("enthalpy")),
    _cp(declareProperty<Real>("cp")),
    _e(declareProperty<Real>("e")),

    _fp(getUserObject<MultiComponentFluidPropertiesPT>("fp"))
{
}

MultiComponentFluidPropertiesMaterialPT::~MultiComponentFluidPropertiesMaterialPT() {}

void
MultiComponentFluidPropertiesMaterialPT::computeQpProperties()
{
  _rho[_qp] = _fp.rho(_pressure[_qp], _temperature[_qp], _xmass[_qp]);
  _h[_qp] = _fp.h(_pressure[_qp], _temperature[_qp], _xmass[_qp]);
  _cp[_qp] = _fp.cp(_pressure[_qp], _temperature[_qp], _xmass[_qp]);
  _e[_qp] = _fp.e(_pressure[_qp], _temperature[_qp], _xmass[_qp]);
}
