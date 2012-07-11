#include "HeatConductionMaterial.h"

template<>
InputParameters validParams<HeatConductionMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addCoupledVar("temp", "Coupled Temperature");

  params.addRequiredParam<Real>("thermal_conductivity", "The thermal conductivity value");
  params.addRequiredParam<Real>("specific_heat", "The specific heat value");

  return params;
}

HeatConductionMaterial::HeatConductionMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),

    _my_thermal_conductivity(getParam<Real>("thermal_conductivity")),
    _my_specific_heat(getParam<Real>("specific_heat")),

    _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
    _thermal_conductivity_dT(declareProperty<Real>("thermal_conductivity_dT")),
    _specific_heat(declareProperty<Real>("specific_heat"))
{}

void
HeatConductionMaterial::computeProperties()
{
  for(unsigned int qp(0); qp < _qrule->n_points(); ++qp)
  {
    _thermal_conductivity[qp] = _my_thermal_conductivity;
    _thermal_conductivity_dT[qp] = 0;
    _specific_heat[qp] = _my_specific_heat;
  }
}
