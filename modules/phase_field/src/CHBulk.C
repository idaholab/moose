#include "CHBulk.h"

template<>
InputParameters validParams<CHBulk>()
{
  InputParameters params = validParams<KernelGrad>();
  params.addParam<std::string>("mob_name","M","The mobility used with the kernel");
  params.addParam<bool>("implicit",true,"The kernel will be run with implicit time integration");
  
  return params;
}

CHBulk::CHBulk(const std::string & name, InputParameters parameters)
  :KernelGrad(name, parameters),
   _mob_name(getParam<std::string>("mob_name")),
   _M(getMaterialProperty<Real>(_mob_name)),
   _implicit(getParam<bool>("implicit"))
{  
}

RealGradient
CHBulk::computeGradDFDCons(PFFunctionType type, Real c, RealGradient grad_c)
{
  switch (type)
  {
  case Residual:
    return 3*c*c*grad_c - grad_c; // return Residual value
    
  case Jacobian: 
    return 6*c*_phi[_j][_qp]*grad_c + 3*c*c*_grad_phi[_j][_qp] - _grad_phi[_j][_qp]; //return Jacobian value
    //return 0.0;
    
  }
  
  mooseError("Invalid type passed in");
}

RealGradient
CHBulk::precomputeQpResidual()
{
  Real c;
  RealGradient grad_c;
  if (_implicit)
  {
    c = _u[_qp];
    grad_c = _grad_u[_qp];
  }
  else
  {
    c = _u_old[_qp];
    grad_c = _grad_u_old[_qp];
  }
  
  return _M[_qp] * computeGradDFDCons(Residual, c, grad_c);
}

RealGradient
CHBulk::precomputeQpJacobian()
{
  Real c;
  RealGradient grad_c;
  if (_implicit)
  {
    c = _u[_qp];
    grad_c = _grad_u[_qp];
  }
  else
  {
    c = _u_old[_qp];
    grad_c = _grad_u_old[_qp];
  }

  return _M[_qp] * computeGradDFDCons(Jacobian, c, grad_c);
}
