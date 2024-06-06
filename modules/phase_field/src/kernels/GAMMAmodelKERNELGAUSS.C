
#include "GAMMAmodelKERNELGAUSS.h"

registerMooseObject("PhaseFieldApp", GAMMAmodelKERNELGAUSS);

InputParameters
GAMMAmodelKERNELGAUSS::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Implements the term:-L\\ m \\nabla \\cdot \\left( \\sum_{j \\neq i} \\left( \\frac{\\partial \\gamma_{ij}}{\\partial \\nabla\\eta_i} \\right) \\eta_i^2 \\eta_j^2 \\right)");

  params.addRequiredCoupledVar("vminus", "Array of coupled order parameter names for other order parameters");

  return params;
}

GAMMAmodelKERNELGAUSS::GAMMAmodelKERNELGAUSS(const InputParameters & parameters)
  : ADKernel(parameters),
    _L_AD(getADMaterialProperty<Real>("L_AD")),

    _m(getADMaterialProperty<Real>("m")),
    _dgammadx(getADMaterialProperty<Real>("dgammadx")),
    _dgammady(getADMaterialProperty<Real>("dgammady")),
    _dgammadz(getADMaterialProperty<Real>("dgammadz")),

    _op_num(coupledComponents("vminus")),
    _vals(adCoupledValues("vminus"))


{
}

ADReal
GAMMAmodelKERNELGAUSS::computeQpResidual()
{

  ADReal gradtestx = (_grad_test[_i][_qp](0));
  ADReal gradtesty = (_grad_test[_i][_qp](1));
  ADReal gradtestz = (_grad_test[_i][_qp](2));

  // Sum all other order parameters.
  ADReal SumEtaj = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    SumEtaj += (*_vals[i])[_qp] * (*_vals[i])[_qp];


  return  (1) * _L_AD[_qp] * _m[_qp] * SumEtaj * _u[_qp] * _u[_qp] * ( (( _dgammadx[_qp]) * gradtestx) + ((_dgammady[_qp]) * gradtesty) + ((_dgammadz[_qp]) * gradtestz) );

}
