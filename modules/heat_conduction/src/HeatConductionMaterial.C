#include "HeatConductionMaterial.h"

template<>
InputParameters validParams<HeatConductionMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addCoupledVar("temp", "Coupled Temperature");
  params.addParam<FunctionName>("thermal_conductivity_temperature_function", "", "Thermal conductivity as a function of temperature.");
  

  params.addRequiredParam<Real>("thermal_conductivity", "The thermal conductivity value");
  params.addRequiredParam<Real>("specific_heat", "The specific heat value");

  return params;
}

HeatConductionMaterial::HeatConductionMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    
    _has_temp(isCoupled("temp")),
    _temperature(_has_temp ? coupledValue("temp") : _zero),
    _my_thermal_conductivity(getParam<Real>("thermal_conductivity")),
    _my_specific_heat(getParam<Real>("specific_heat")),

    _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
    _thermal_conductivity_dT(declareProperty<Real>("thermal_conductivity_dT")),
    _specific_heat(declareProperty<Real>("specific_heat")),
    _thermal_conductivity_temperature_function( getParam<FunctionName>("thermal_conductivity_temperature_function") != "" ? &getFunction("thermal_conductivity_temperature_function") : NULL)
    
{
  if (_thermal_conductivity_temperature_function && !_has_temp)
    mooseError("Must couple with temperature if using thermal conductivity function");
}

void
HeatConductionMaterial::computeProperties()
{
  for(unsigned int qp(0); qp < _qrule->n_points(); ++qp)
  {
    if (_thermal_conductivity_temperature_function)
    {
      Point p;
      _thermal_conductivity[qp] = _thermal_conductivity_temperature_function->value(_temperature[qp], p);
      _thermal_conductivity_dT[qp] = 0;
    }

    else
    {
      _thermal_conductivity[qp] = _my_thermal_conductivity;
      _thermal_conductivity_dT[qp] = 0;
    }
    _specific_heat[qp] = _my_specific_heat;
    
  }
}
