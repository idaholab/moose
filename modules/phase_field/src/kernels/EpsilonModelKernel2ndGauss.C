#include "EpsilonModelKernel2ndGauss.h"

registerMooseObject("PhaseFieldApp", EpsilonModelKernel2ndGauss);

InputParameters
EpsilonModelKernel2ndGauss::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Implements the term: -L \\nabla \\cdot \\left( \\frac{\\partial "
                             "m(\\theta, v)}{\\partial \\nabla \\eta_i} f_0 \\right)");

  params.addParam<MaterialPropertyName>("f_name", "F", "Base name of the free energy function");

  return params;
}

EpsilonModelKernel2ndGauss::EpsilonModelKernel2ndGauss(const InputParameters & parameters)
  : ADKernel(parameters),
    _L_AD(getADMaterialProperty<Real>("L_AD")),
    _dm_plus(getADMaterialProperty<RealGradient>("dm_plus")),
    _F(getMaterialProperty<Real>("F"))
{
}

ADReal
EpsilonModelKernel2ndGauss::computeQpResidual()
{
  const auto & grad_test = _grad_test[_i][_qp];

  return _L_AD[_qp] * _F[_qp] * _dm_plus[_qp] * grad_test;
}
