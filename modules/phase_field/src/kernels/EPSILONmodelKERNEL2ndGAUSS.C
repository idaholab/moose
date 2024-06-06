
#include "EPSILONmodelKERNEL2ndGAUSS.h"

registerMooseObject("PhaseFieldApp", EPSILONmodelKERNEL2ndGAUSS);

InputParameters
EPSILONmodelKERNEL2ndGAUSS::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Implements the term:-L\\nabla \\cdot \\left( \\frac{\\partial m(\\theta, v)}{\\partial \\nabla\\eta_i} f_0 \\right)");

  params.addParam<MaterialPropertyName>("f_name", "F", " Base name of the free energy function");


  return params;
}

EPSILONmodelKERNEL2ndGAUSS::EPSILONmodelKERNEL2ndGAUSS(const InputParameters & parameters)
  : ADKernel(parameters),
    _L_AD(getADMaterialProperty<Real>("L_AD")),
    
    _dmdx(getADMaterialProperty<Real>("dmdx")),
    _dmdy(getADMaterialProperty<Real>("dmdy")),
    _dmdz(getADMaterialProperty<Real>("dmdz")),

    _F(getMaterialProperty<Real>("F"))

{
}

ADReal
EPSILONmodelKERNEL2ndGAUSS::computeQpResidual()
{

  ADReal gradtestx = (_grad_test[_i][_qp](0));
  ADReal gradtesty = (_grad_test[_i][_qp](1));
  ADReal gradtestz = (_grad_test[_i][_qp](2));

  return  _L_AD[_qp] * (_F[_qp]) * ( (( _dmdx[_qp]) * gradtestx) + ((_dmdy[_qp]) * gradtesty) + ((_dmdz[_qp]) * gradtestz) );
}
