#include "SolidMaterial.h"

template <>
InputParameters
validParams<SolidMaterial>()
{
  InputParameters params = validParams<Material>();
  // Coupled variables
  params.addRequiredCoupledVar("temperature", "Temperature in the solid");
  params.addRequiredParam<UserObjectName>(
      "properties", "The name of an user object describing material conductivity");
  return params;
}

SolidMaterial::SolidMaterial(const InputParameters & parameters)
  : Material(parameters),
    _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
    _specific_heat(declareProperty<Real>("specific_heat")),
    _density(declareProperty<Real>("density")),
    _temp(coupledValue("temperature")),
    _props(getUserObject<SolidMaterialProperties>("properties"))
{
}

void
SolidMaterial::computeQpProperties()
{
  _thermal_conductivity[_qp] = _props.k(_temp[_qp]);
  _specific_heat[_qp] = _props.Cp(_temp[_qp]);
  _density[_qp] = _props.rho(_temp[_qp]);
}
