
#include "EpsilonModelKernel2ndV2Gauss.h"

registerMooseObject("PhaseFieldApp", EpsilonModelKernel2ndV2Gauss);

InputParameters
EpsilonModelKernel2ndV2Gauss::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Implements the term:-L\\nabla \\cdot \\left( \\frac{\\partial "
                             "m(\\theta, v)}{\\partial \\nabla\\eta_i} f_0 \\right)");

  params.addParam<MaterialPropertyName>("f_name", "F", " Base name of the free energy function");

  return params;
}

EpsilonModelKernel2ndV2Gauss::EpsilonModelKernel2ndV2Gauss(const InputParameters & parameters)
  : ADKernel(parameters),
    _L_AD(getADMaterialProperty<Real>("L_AD")),

    _dmdxplus(getADMaterialProperty<Real>("dmdxplus")),
    _dmdyplus(getADMaterialProperty<Real>("dmdyplus")),
    _dmdzplus(getADMaterialProperty<Real>("dmdzplus")),

    _F(getMaterialProperty<Real>("F"))

{
}

ADReal
EpsilonModelKernel2ndV2Gauss::computeQpResidual()
{

  ADReal gradtestx = (_grad_test[_i][_qp](0));
  ADReal gradtesty = (_grad_test[_i][_qp](1));
  ADReal gradtestz = (_grad_test[_i][_qp](2));

  return _L_AD[_qp] * (_F[_qp]) *
         (((_dmdxplus[_qp]) * gradtestx) + ((_dmdyplus[_qp]) * gradtesty) +
          ((_dmdzplus[_qp]) * gradtestz));
}
