#include "CHSplit1.h"

template<>
InputParameters validParams<CHSplit1>()
{
  InputParameters params = validParams<Kernel>();
     
  params.addParam<std::string>("mob_name","M","The mobility used with the kernel");
  params.addRequiredCoupledVar("w","intermediate parameter--chemical potential");

  return params;
}

CHSplit1::CHSplit1(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
//  This _w_var is needed to compute off-diagonal Jacobian.
   _w_var(coupled("w")),
   _grad_w(coupledGradient("w")),
   _mob_name(getParam<std::string>("mob_name")),
   _M(getMaterialProperty<Real>(_mob_name))
{}

Real
CHSplit1::computeQpResidual()
{
 
  return  _M[_qp]*_grad_w[_qp] * _grad_test[_i][_qp];
}

Real
CHSplit1::computeQpJacobian()
{
  return 0;
}

Real
CHSplit1::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _w_var)
  {

   return _M[_qp]*_grad_phi[_j][_qp] * _grad_test[_i][_qp];

  }

  return 0.0;
}

