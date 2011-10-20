#include "CHSplit1.h"

template<>
InputParameters validParams<CHSplit1>()
{
  InputParameters params = validParams<Kernel>();
     
  params.addParam<std::string>("mob_name","M","The mobility used with the kernel");
  params.addRequiredCoupledVar("c","intermediate parameter--concentration");
  params.addParam<bool>("implicit",true,"The time integration is implicit");

  return params;
}

CHSplit1::CHSplit1(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _c_var(coupled("c")),
   _mob_name(getParam<std::string>("mob_name")),
   _M(getMaterialProperty<Real>(_mob_name)),
   _implicit(getParam<bool>("implicit"))
{}

Real
CHSplit1::computeQpResidual()
{
  RealGradient grad_w = _grad_u[_qp];
  if (_implicit)
    grad_w = _grad_u_old[_qp];
  
  return  _M[_qp]*grad_w*_grad_test[_i][_qp];
}

Real
CHSplit1::computeQpJacobian()
{
  if (_implicit)
    return _M[_qp]*_grad_phi[_j][_qp] * _grad_test[_i][_qp];
  else
    return 0.0;
  
}


Real
CHSplit1::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.0;
}

