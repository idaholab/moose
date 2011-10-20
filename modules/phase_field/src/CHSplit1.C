#include "CHSplit1.h"

template<>
InputParameters validParams<CHSplit1>()
{
  InputParameters params = validParams<Kernel>();
     
  params.addParam<std::string>("mob_name","M","The mobility used with the kernel");
  params.addRequiredCoupledVar("c","intermediate parameter--concentration");

  return params;
}

CHSplit1::CHSplit1(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
//  This _w_var is needed to compute off-diagonal Jacobian.
   _c_var(coupled("c")),
   _mob_name(getParam<std::string>("mob_name")),
   _M(getMaterialProperty<Real>(_mob_name))
{}

Real
CHSplit1::computeQpResidual()
{
 
  return  _M[_qp]*_grad_u[_qp] * _grad_test[_i][_qp];
}

Real
CHSplit1::computeQpJacobian()
{
  return _M[_qp]*_grad_phi[_j][_qp] * _grad_test[_i][_qp];
}

Real
CHSplit1::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _c_var)
  {

    return 0.0;
    

  }

  return 0.0;
}

