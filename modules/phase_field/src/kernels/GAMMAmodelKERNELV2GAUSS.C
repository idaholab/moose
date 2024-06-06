
#include "GAMMAmodelKERNELV2GAUSS.h"

registerMooseObject("PhaseFieldApp", GAMMAmodelKERNELV2GAUSS);

InputParameters
GAMMAmodelKERNELV2GAUSS::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Implements the term:-L\\ m \\nabla \\cdot \\left( \\sum_{j \\neq i} \\left( \\frac{\\partial \\gamma_{ij}}{\\partial \\nabla\\eta_i} \\right) \\eta_i^2 \\eta_j^2 \\right)");

  params.addRequiredCoupledVar("vplus", "Array of coupled order parameter names for other order parameters");

  return params;
}

GAMMAmodelKERNELV2GAUSS::GAMMAmodelKERNELV2GAUSS(const InputParameters & parameters)
  : ADKernel(parameters),
    _L_AD(getADMaterialProperty<Real>("L_AD")),

    _m(getADMaterialProperty<Real>("m")),
    _dgammadxplus(getADMaterialProperty<Real>("dgammadxplus")),
    _dgammadyplus(getADMaterialProperty<Real>("dgammadyplus")),
    _dgammadzplus(getADMaterialProperty<Real>("dgammadzplus")),

    _op_num(coupledComponents("vplus")),
    _vals(adCoupledValues("vplus"))


{
}

ADReal
GAMMAmodelKERNELV2GAUSS::computeQpResidual()
{

  ADReal gradtestx = (_grad_test[_i][_qp](0));
  ADReal gradtesty = (_grad_test[_i][_qp](1));
  ADReal gradtestz = (_grad_test[_i][_qp](2));

  // Sum all other order parameters.
  ADReal SumEtaj = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    SumEtaj += (*_vals[i])[_qp] * (*_vals[i])[_qp];


  return  (1) * _L_AD[_qp] * _m[_qp] * SumEtaj * _u[_qp] * _u[_qp] * ( (( _dgammadxplus[_qp]) * gradtestx) + ((_dgammadyplus[_qp]) * gradtesty) + ((_dgammadzplus[_qp]) * gradtestz) );

}
