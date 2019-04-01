#include "LapidusCoefMaterial.h"

registerMooseObject("THMApp", LapidusCoefMaterial);

template <>
InputParameters
validParams<LapidusCoefMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<Real>("cl", "User-specified coefficient");
  params.addRequiredCoupledVar("vel", "The velocity of the kth phase, aux variable");
  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>(
      "coef_name", "The material property name that will hold the dissipation coefficient");
  return params;
}

LapidusCoefMaterial::LapidusCoefMaterial(const InputParameters & parameters)
  : Material(parameters),
    _velocity_grad(coupledGradient("vel")),
    _cl(getParam<Real>("cl")),
    _dir(getMaterialProperty<RealVectorValue>("direction")),
    _coef(declareProperty<Real>(getParam<MaterialPropertyName>("coef_name")))
{
}

void
LapidusCoefMaterial::computeQpProperties()
{
  Real h = _current_elem->hmax();
  _coef[_qp] = _cl * h * h * std::fabs(_velocity_grad[_qp] * _dir[_qp]);
}
