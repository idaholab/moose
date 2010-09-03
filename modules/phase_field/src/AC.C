#include "AC.h"

template<>
InputParameters validParams<AC>()
{
  InputParameters params = validParams<Kernel>();
  
  return params;
}

AC::AC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _kappa(getMaterialProperty<Real>("kappa_op")),
   _L(getMaterialProperty<Real>("L"))
{ 
}

Real
AC::computeDFDOP(PFFunctionType type)
{
  switch (type)
  {
  case Residual:
    return _u[_qp]*_u[_qp]*_u[_qp] - _u[_qp] ;

  case Jacobian:
    return _phi[_j][_qp]*(3*_u[_qp]*_u[_qp] - 1. );
  }
    
}

Real
AC::computeQpResidual()
{
  Real dFdeta = computeDFDOP(Residual);
  
  return  _L[_qp]*(dFdeta*_test[_i][_qp]) + _kappa[_qp]*_L[_qp]*( _grad_u[_qp] * _grad_test[_i][_qp]);
}

Real
AC::computeQpJacobian()
{
  Real dFdeta = computeDFDOP(Jacobian);
  
  return _L[_qp]*(dFdeta*_test[_i][_qp]) + _kappa[_qp]*_L[_qp]*( _grad_phi[_j][_qp] * _grad_test[_i][_qp]);
}
