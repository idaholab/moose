#include "SolidMaterial.h"

template<>
InputParameters validParams<SolidMaterial>()
{
  InputParameters params = validParams<Material>();
  // Coupled variables
  params.addRequiredCoupledVar("temperature", "Temperature in the solid");
  params.addRequiredParam<FunctionName>("k", "Function of temperature describing thermal conductivity");
  params.addRequiredParam<FunctionName>("Cp", "Function of temperature describing specific heat");
  params.addRequiredParam<FunctionName>("rho", "Function of temperature describing density");
  return params;
}

SolidMaterial::SolidMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
    _specific_heat(declareProperty<Real>("specific_heat")),
    _density(declareProperty<Real>("density")),
    _temp(coupledValue("temperature")),
    _k(getFunction("k")),
    _Cp(getFunction("Cp")),
    _rho(getFunction("rho"))
{
}

void SolidMaterial::computeQpProperties()
{
  Point pt;
  _thermal_conductivity[_qp] = _k.value(_temp[_qp], pt);
  _specific_heat[_qp] = _Cp.value(_temp[_qp], pt);
  _density[_qp] = _rho.value(_temp[_qp], pt);
}
