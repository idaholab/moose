
#include "EPSILONmodelKERNEL1stGAUSS.h"

registerMooseObject("PhaseFieldApp", EPSILONmodelKERNEL1stGAUSS);

InputParameters
EPSILONmodelKERNEL1stGAUSS::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Implements the term:-L\\nabla \\cdot \\left( \\frac{1}{2} \\frac{\\partial \\epsilon(\\theta, v)}{\\partial \\nabla\\eta_i} \\sum_{i=1}^p (\\eta_i^2) \\right)");

  params.addRequiredCoupledVar("vminus", "Array of coupled order parameter names for other order parameters");

  return params;
}

EPSILONmodelKERNEL1stGAUSS::EPSILONmodelKERNEL1stGAUSS(const InputParameters & parameters)
  : ADKernel(parameters),
    _L_AD(getADMaterialProperty<Real>("L_AD")),
    
    _eps(getADMaterialProperty<Real>("eps")),
    _depsdx(getADMaterialProperty<Real>("depsdx")),
    _depsdy(getADMaterialProperty<Real>("depsdy")),
    _depsdz(getADMaterialProperty<Real>("depsdz")),

    _op_num(coupledComponents("vminus")),
    _vals(adCoupledValues("vminus")),
    _grad_vals(adCoupledGradients("vminus"))

{
}

ADReal
EPSILONmodelKERNEL1stGAUSS::computeQpResidual()
{
  ADReal gradux2 = (_grad_u[_qp](0)) * (_grad_u[_qp](0));
  ADReal graduy2 = (_grad_u[_qp](1)) * (_grad_u[_qp](1));
  ADReal graduz2 = (_grad_u[_qp](2)) * (_grad_u[_qp](2));
  ADReal SqrGrad = gradux2 + graduy2 + graduz2;
  ADReal gradtestx = (_grad_test[_i][_qp](0));
  ADReal gradtesty = (_grad_test[_i][_qp](1));
  ADReal gradtestz = (_grad_test[_i][_qp](2));
  ADReal Grad = (_grad_u[_qp](0)) + (_grad_u[_qp](1)) + (_grad_u[_qp](2));


  ADReal SumSqrGradEtaj = 0.0;
  ADReal Sumsq = 0.0;
  ADReal sqx = 0.0;
  ADReal sqy = 0.0;
  ADReal sqz = 0.0;


  for (unsigned int i = 0; i < _op_num; ++i)
    SumSqrGradEtaj += (*_grad_vals[i])[_qp] * (*_grad_vals[i])[_qp];

    return  _L_AD[_qp] * (0.5) * (SqrGrad + SumSqrGradEtaj) * ( (( _depsdx[_qp]) * gradtestx) + ((_depsdy[_qp]) * gradtesty) + ((_depsdz[_qp]) * gradtestz) );
}
