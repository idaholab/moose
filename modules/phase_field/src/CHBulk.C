#include "CHBulk.h"

template<>
InputParameters validParams<CHBulk>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<std::string>("mob_name","M","The mobility used with the kernel");
  
  return params;
}

CHBulk::CHBulk(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _mob_name(getParam<std::string>("mob_name")),
   _M(getMaterialProperty<Real>(_mob_name))
{  
}

RealGradient
CHBulk::computeGradDFDCons(PFFunctionType type)
{
  switch (type)
  {
  case Residual:
    return 3*_u[_qp]*_u[_qp]*_grad_u[_qp] - _grad_u[_qp]; // return Residual value
    
  case Jacobian: 
    return 6*_u[_qp]*_phi[_j][_qp]*_grad_u[_qp] + 3*_u[_qp]*_u[_qp]*_grad_phi[_j][_qp] - _grad_phi[_j][_qp]; //return Jacobian value
    
  }
  
  mooseError("Invalid type passed in");
}

Real
CHBulk::computeQpResidual()
{
  RealGradient grad_dfdc = computeGradDFDCons(Residual);
    
  return _M[_qp] * (grad_dfdc * _grad_test[_i][_qp]);
}

Real
CHBulk::computeQpJacobian()
{
  RealGradient Jac_grad_dfdc = computeGradDFDCons(Jacobian);
  
  return _M[_qp] * (Jac_grad_dfdc * _grad_test[_i][_qp]);
}
