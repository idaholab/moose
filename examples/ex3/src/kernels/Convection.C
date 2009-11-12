#include "Convection.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
template<>
InputParameters valid_params<Convection>()
{
  InputParameters params;
  return params;
}

Real Convection::computeQpResidual()
{
  // _grad_some_var[_qp] * _grad_u[_qp] is actually doing a dot product
  return _phi[_i][_qp]*(_grad_some_var[_qp]*_grad_u[_qp]);
}

Real Convection::computeQpJacobian()
{
  // the partial derivative of _grad_u is just _dphi[_j]
  return _phi[_i][_qp]*(_grad_some_var[_qp]*_dphi[_j][_qp]);
}
