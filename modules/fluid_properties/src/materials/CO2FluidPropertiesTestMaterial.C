/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "CO2FluidPropertiesTestMaterial.h"

template<>
InputParameters validParams<CO2FluidPropertiesTestMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("pressure", "Pressure (Pa)");
  params.addRequiredCoupledVar("temperature", "Temperature");
  params.addParam<bool>("sublimation", false, "Flag to calculate sublimation pressure");
  params.addParam<bool>("melting", false, "Flag to calculate melting pressure");
  params.addParam<bool>("vapor", false, "Flag to calculate vapor pressure");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  return params;
}

CO2FluidPropertiesTestMaterial::CO2FluidPropertiesTestMaterial(const InputParameters & parameters) :
    Material(parameters),
    _pressure(coupledValue("pressure")),
    _temperature(coupledValue("temperature")),

    _psub(declareProperty<Real>("sublimation_pressure")),
    _pmelt(declareProperty<Real>("melting_pressure")),
    _pvap(declareProperty<Real>("vapor_pressure")),
    _rhovap(declareProperty<Real>("saturated_vapor_density")),
    _rhosat(declareProperty<Real>("saturated_liquid_density")),
    _rhopartial(declareProperty<Real>("partial_density")),

    _fp(getUserObject<CO2FluidProperties>("fp")),
    _sublimation(getParam<bool>("sublimation")),
    _melting(getParam<bool>("melting")),
    _vapor(getParam<bool>("vapor"))
{
}

CO2FluidPropertiesTestMaterial::~CO2FluidPropertiesTestMaterial()
{
}

void
CO2FluidPropertiesTestMaterial::computeQpProperties()
{
  // Optionally calculated material properties
  _psub[_qp] = (_sublimation ? _fp.sublimationPressure(_temperature[_qp]) : 0.0);
  _pmelt[_qp] = (_melting ? _fp.meltingPressure(_temperature[_qp]) : 0.0);
  _pvap[_qp] = (_vapor ? _fp.vaporPressure(_temperature[_qp]) : 0.0);
  _rhovap[_qp] = (_vapor ? _fp.saturatedVaporDensity(_temperature[_qp]) : 0.0);
  _rhosat[_qp] = (_vapor ? _fp.saturatedLiquidDensity(_temperature[_qp]) : 0.0);
  _rhopartial[_qp] = _fp.partialDensity(_temperature[_qp]);
}
