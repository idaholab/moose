#include "SolidMaterial.h"

registerMooseObject("RELAP7App", SolidMaterial);

template <>
InputParameters
validParams<SolidMaterial>()
{
  InputParameters params = validParams<Material>();
  // Coupled variables
  params.addRequiredCoupledVar("T", "Temperature in the solid");

  params.addRequiredParam<UserObjectName>(
      "properties", "The name of an user object describing material conductivity");
  return params;
}

SolidMaterial::SolidMaterial(const InputParameters & parameters)
  : Material(parameters),
    _thermal_conductivity(declareProperty<Real>("k_solid")),
    _specific_heat(declareProperty<Real>("cp_solid")),
    _density(declareProperty<Real>("rho_solid")),
    _temp(coupledValue("T")),
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
