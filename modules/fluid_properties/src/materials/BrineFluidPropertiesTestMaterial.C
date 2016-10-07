/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "BrineFluidPropertiesTestMaterial.h"

template<>
InputParameters validParams<BrineFluidPropertiesTestMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("pressure", "pressure (Pa)");
  params.addRequiredCoupledVar("temperature", "temperature (K)");
  params.addRequiredCoupledVar("xnacl", "NaCl mass fraction (-)");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addParam<bool>("vapor", false, "Flag to calculate vapor pressure");
  params.addClassDescription("Material to test brine properties");
  return params;
}

BrineFluidPropertiesTestMaterial::BrineFluidPropertiesTestMaterial(const InputParameters & parameters) :
    Material(parameters),
    _pressure(coupledValue("pressure")),
    _temperature(coupledValue("temperature")),
    _xnacl(coupledValue("xnacl")),

    _rho(declareProperty<Real>("density")),
    _mu(declareProperty<Real>("viscosity")),
    _h(declareProperty<Real>("enthalpy")),
    _cp(declareProperty<Real>("cp")),
    _e(declareProperty<Real>("e")),
    _k(declareProperty<Real>("k")),
    _solubility(declareProperty<Real>("solubility")),
    _pvap(declareProperty<Real>("vapor")),
    _rho_halite(declareProperty<Real>("halite_density")),
    _cp_halite(declareProperty<Real>("halite_cp")),
    _h_halite(declareProperty<Real>("halite_enthalpy")),
    _vapor(getParam<bool>("vapor")),

    _fp(getUserObject<BrineFluidProperties>("fp")),
    _water_fp(_fp.getComponent(BrineFluidProperties::WATER)),
    _halite_fp(_fp.getComponent(BrineFluidProperties::NACL))
{
}

BrineFluidPropertiesTestMaterial::~BrineFluidPropertiesTestMaterial()
{
}

void
BrineFluidPropertiesTestMaterial::computeQpProperties()
{
  _rho[_qp] = _fp.rho(_pressure[_qp], _temperature[_qp], _xnacl[_qp]);
  _h[_qp] = _fp.h(_pressure[_qp], _temperature[_qp], _xnacl[_qp]);
  _cp[_qp] = _fp.cp(_pressure[_qp], _temperature[_qp], _xnacl[_qp]);
  _e[_qp] = _fp.e(_pressure[_qp], _temperature[_qp], _xnacl[_qp]);
  _solubility[_qp] = _fp.haliteSolubility(_temperature[_qp]);
  _pvap[_qp] = (_vapor ? _fp.pSat(_temperature[_qp], _xnacl[_qp]) : 0.0);
  _rho_halite[_qp] = _halite_fp.rho(_pressure[_qp], _temperature[_qp]);
  _cp_halite[_qp] = _halite_fp.cp(_pressure[_qp], _temperature[_qp]);
  _h_halite[_qp] = _halite_fp.h(_pressure[_qp], _temperature[_qp]);

  // Calculate the water density as it is used to calculate viscosity and
  // thermal conductivity
  Real rhow = _water_fp.rho(_pressure[_qp], _temperature[_qp]);
  _mu[_qp] = _fp.mu(rhow, _temperature[_qp], _xnacl[_qp]);
  _k[_qp] = _fp.k(rhow, _temperature[_qp], _xnacl[_qp]);
}
