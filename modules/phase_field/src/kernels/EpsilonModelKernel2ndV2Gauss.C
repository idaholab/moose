#include "EpsilonModelKernel2ndV2Gauss.h"

registerMooseObject("PhaseFieldApp", EpsilonModelKernel2ndV2Gauss);

InputParameters
EpsilonModelKernel2ndV2Gauss::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Implements the term: -L \\nabla \\cdot \\left( \\frac{\\partial "
                             "m(\\theta, v)}{\\partial \\nabla \\eta_i} f_0 \\right)");

  params.addParam<MaterialPropertyName>("f_name", "F", "Base name of the free energy function");

  return params;
}

EpsilonModelKernel2ndV2Gauss::EpsilonModelKernel2ndV2Gauss(const InputParameters & parameters)
  : ADKernel(parameters),
    _L_AD(getADMaterialProperty<Real>("L_AD")),
    _dm_minus(getADMaterialProperty<RealGradient>("dm_minus")),
    _F(getMaterialProperty<Real>("F"))
{
}

ADReal
EpsilonModelKernel2ndV2Gauss::computeQpResidual()
{
  const auto & grad_test = _grad_test[_i][_qp];

  return _L_AD[_qp] * _F[_qp] * _dm_minus[_qp] * grad_test;
}
