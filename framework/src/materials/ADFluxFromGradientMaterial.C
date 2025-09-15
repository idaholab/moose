#include "ADFluxFromGradientMaterial.h"

registerMooseObject("MooseApp", ADFluxFromGradientMaterial);

InputParameters
ADFluxFromGradientMaterial::validParams()
{
  InputParameters params = Material::validParams();

  // Required name for the output flux property
  params.addRequiredParam<MaterialPropertyName>("flux", "Name of the flux material property");

  // Coupled variable whose gradient is used
  params.addRequiredCoupledVar("u", "Variable used to compute the flux");

  // Optional name for the diffusivity material property (default: 'diffusivity')
  params.addParam<MaterialPropertyName>(
      "diffusivity", "diffusivity", "The diffusivity material property name");

  return params;
}

ADFluxFromGradientMaterial::ADFluxFromGradientMaterial(const InputParameters & parameters)
  : Material(parameters),
    _grad_u(adCoupledGradient("u")),
    _diffusivity(getADMaterialProperty<Real>(getParam<MaterialPropertyName>("diffusivity"))),
    _flux(declareADProperty<RealVectorValue>(getParam<MaterialPropertyName>("flux")))
{
}

void
ADFluxFromGradientMaterial::computeQpProperties()
{
  _flux[_qp] = -_diffusivity[_qp] * _grad_u[_qp];
}
