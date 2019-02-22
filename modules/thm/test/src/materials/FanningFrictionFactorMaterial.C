#include "FanningFrictionFactorMaterial.h"

registerMooseObject("RELAP7TestApp", FanningFrictionFactorMaterial);

template <>
InputParameters
validParams<FanningFrictionFactorMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<MaterialPropertyName>(
      "f_F", "Name to give Fanning friction factor material property");
  params.addRequiredParam<MaterialPropertyName>("f_D", "Darcy friction factor material property");

  params.addClassDescription("Computes Fanning friction factor from Darcy friction factor");

  return params;
}

FanningFrictionFactorMaterial::FanningFrictionFactorMaterial(const InputParameters & parameters)
  : Material(parameters),

    _f_D(getMaterialProperty<Real>("f_D")),
    _f_F(declareProperty<Real>(getParam<MaterialPropertyName>("f_F")))
{
}

void
FanningFrictionFactorMaterial::computeQpProperties()
{
  _f_F[_qp] = 0.25 * _f_D[_qp];
}
