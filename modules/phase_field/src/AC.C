#include "AC.h"

template<>
InputParameters validParams<AC>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<std::string>("mob_name","L","The mobility used with the kernel");
  params.addParam<std::string>("kappa_name","kappa_op","The kappa used with the kernel");
  
  return params;
}

AC::AC(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _mob_name(getParam<std::string>("mob_name")),
   _kappa_name(getParam<std::string>("kappa_name")),
   _kappa(getMaterialProperty<Real>(_kappa_name)),
   _L(getMaterialProperty<Real>(_mob_name))
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

  mooseError("Invalid type passed in");
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
