#include "Convection.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
template<>
InputParameters validParams<Convection>()
{
  InputParameters params;
  params.set<Real>("x")=0.0;
  params.set<Real>("y")=0.0;
  params.set<Real>("z")=0.0;
  return params;
}

Real Convection::computeQpResidual()
{
  // velocity * _grad_u[_qp] is actually doing a dot product
  return _phi[_i][_qp]*(velocity*_grad_u[_qp]);
}

Real Convection::computeQpJacobian()
{
  // the partial derivative of _grad_u is just _dphi[_j]
  return _phi[_i][_qp]*(velocity*_dphi[_j][_qp]);
}
