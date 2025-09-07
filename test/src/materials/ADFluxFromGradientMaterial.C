#include "ADFluxFromGradientMaterial.h"

registerMooseObject("MooseTestApp", ADFluxFromGradientMaterial);

InputParameters
ADFluxFromGradientMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("flux", "Name of the flux material property");
  params.addRequiredCoupledVar("u", "Variable used to compute the flux");
  params.addParam<Real>("diffusivity", 1.0, "Diffusivity multiplying the gradient");
  return params;
}

ADFluxFromGradientMaterial::ADFluxFromGradientMaterial(const InputParameters & parameters)
  : Material(parameters),
    _grad_u(adCoupledGradient("u")),
    _diffusivity(getParam<Real>("diffusivity")),
    _flux(declareADProperty<RealVectorValue>(getParam<MaterialPropertyName>("flux")))
{
}

void
ADFluxFromGradientMaterial::computeQpProperties()
{
  _flux[_qp] = -_diffusivity * _grad_u[_qp];
}
