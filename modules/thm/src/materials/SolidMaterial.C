#include "SolidMaterial.h"
#include "HeatConductionModel.h"

registerMooseObject("THMApp", SolidMaterial);

InputParameters
SolidMaterial::validParams()
{
  InputParameters params = Material::validParams();
  // Coupled variables
  params.addRequiredCoupledVar("T", "Temperature in the solid");

  params.addRequiredParam<UserObjectName>(
      "properties", "The name of an user object describing material conductivity");
  return params;
}

SolidMaterial::SolidMaterial(const InputParameters & parameters)
  : Material(parameters),
    _thermal_conductivity(declareProperty<Real>(HeatConductionModel::THERMAL_CONDUCTIVITY)),
    _specific_heat(declareProperty<Real>(HeatConductionModel::SPECIFIC_HEAT_CONSTANT_PRESSURE)),
    _density(declareProperty<Real>(HeatConductionModel::DENSITY)),
    _temp(coupledValue("T")),
    _props(getUserObject<SolidMaterialProperties>("properties"))
{
}

void
SolidMaterial::computeQpProperties()
{
  _thermal_conductivity[_qp] = _props.k(_temp[_qp]);
  _specific_heat[_qp] = _props.cp(_temp[_qp]);
  _density[_qp] = _props.rho(_temp[_qp]);
}
