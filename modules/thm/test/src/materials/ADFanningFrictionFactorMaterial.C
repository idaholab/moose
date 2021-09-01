#include "ADFanningFrictionFactorMaterial.h"

registerMooseObject("THMTestApp", ADFanningFrictionFactorMaterial);

InputParameters
ADFanningFrictionFactorMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<MaterialPropertyName>(
      "f_F", "Name to give Fanning friction factor material property");
  params.addRequiredParam<MaterialPropertyName>("f_D", "Darcy friction factor material property");

  params.addClassDescription("Computes Fanning friction factor from Darcy friction factor");

  return params;
}

ADFanningFrictionFactorMaterial::ADFanningFrictionFactorMaterial(const InputParameters & parameters)
  : Material(parameters),

    _f_D(getADMaterialProperty<Real>("f_D")),
    _f_F(declareADProperty<Real>(getParam<MaterialPropertyName>("f_F")))
{
}

void
ADFanningFrictionFactorMaterial::computeQpProperties()
{
  _f_F[_qp] = 0.25 * _f_D[_qp];
}
