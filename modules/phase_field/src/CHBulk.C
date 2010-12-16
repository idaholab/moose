#include "CHBulk.h"

template<>
InputParameters validParams<CHBulk>()
{
  InputParameters params = validParams<KernelGrad>();
  params.addParam<std::string>("mob_name","M","The mobility used with the kernel");
  
  return params;
}

CHBulk::CHBulk(const std::string & name, InputParameters parameters)
  :KernelGrad(name, parameters),
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
    //return 0.0;
    
  }
  
  mooseError("Invalid type passed in");
}

RealGradient
CHBulk::precomputeQpResidual()
{
  return _M[_qp] * computeGradDFDCons(Residual);
}

RealGradient
CHBulk::precomputeQpJacobian()
{
  return _M[_qp] * computeGradDFDCons(Jacobian);
}
