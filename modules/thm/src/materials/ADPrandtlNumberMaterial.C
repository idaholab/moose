#include "ADPrandtlNumberMaterial.h"
#include "Numerics.h"

registerMooseObject("THMApp", ADPrandtlNumberMaterial);

InputParameters
ADPrandtlNumberMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("cp", "Constant-pressure specific heat");
  params.addRequiredParam<MaterialPropertyName>("mu", "Dynamic viscosity");
  params.addRequiredParam<MaterialPropertyName>("k", "Thermal conductivity");
  params.addClassDescription("Computes Prandtl number as material property");
  return params;
}

ADPrandtlNumberMaterial::ADPrandtlNumberMaterial(const InputParameters & parameters)
  : Material(parameters),
    _Pr(declareADProperty<Real>("Pr")),
    _cp(getADMaterialProperty<Real>("cp")),
    _mu(getADMaterialProperty<Real>("mu")),
    _k(getADMaterialProperty<Real>("k"))
{
}

void
ADPrandtlNumberMaterial::computeQpProperties()
{
  _Pr[_qp] = THM::Prandtl(_cp[_qp], _mu[_qp], _k[_qp]);
}
