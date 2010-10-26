#include "ACBulk.h"

template<>
InputParameters validParams<ACBulk>()
{
  InputParameters params = validParams<KernelValue>();
  params.addParam<std::string>("mob_name","L","The mobility used with the kernel");
  
  return params;
}

ACBulk::ACBulk(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :KernelValue(name, moose_system, parameters),
   _mob_name(getParam<std::string>("mob_name")),
   _L(getMaterialProperty<Real>(_mob_name))
{ 
}

Real
ACBulk::computeDFDOP(PFFunctionType type)
{
  switch (type)
  {
  case Residual:
    return _u[_qp]*_u[_qp]*_u[_qp] - _u[_qp] ;

  case Jacobian:
    return _phi[_j][_qp]*(3*_u[_qp]*_u[_qp] - 1. );
  }

  mooseError("Invalid type passed in");
}

Real
ACBulk::precomputeQpResidual()
{
  Real dFdeta = computeDFDOP(Residual);
  
  return  _L[_qp]*(dFdeta) ;
}

Real
ACBulk::precomputeQpJacobian()
{
  Real dFdeta = computeDFDOP(Jacobian);
  
  return _L[_qp]*(dFdeta);
}
