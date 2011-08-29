#include "NSMassBC.h"

template<>
InputParameters validParams<NSMassBC>()
{
  InputParameters params = validParams<NSIntegratedBC>();
  
  
  return params;
}



NSMassBC::NSMassBC(const std::string & name, InputParameters parameters)
    : NSIntegratedBC(name, parameters)
{
}



Real NSMassBC::computeQpResidual()
{
  // (rho*u.n) phi_i
  RealVectorValue mom(_rho_u[_qp], _rho_v[_qp], _rho_w[_qp]);

  return (mom*_normals[_qp]) * _test[_i][_qp];
}



Real NSMassBC::computeQpJacobian()
{
  // The derivative wrt rho is zero!
  return 0.;
}



Real NSMassBC::computeQpOffDiagJacobian(unsigned jvar)
{
  // Map jvar into the variable m for our problem, regardless of
  // how Moose has numbered things. 
  unsigned m = 99;
  
  if (jvar == _rho_var_number)
    m = 0;
  else if (jvar == _rhou_var_number)
    m = 1;
  else if (jvar == _rhov_var_number)
    m = 2;
  else if (jvar == _rhow_var_number)
    m = 3;
  else if (jvar == _rhoe_var_number)
    m = 4;
  else
    mooseError("Invalid jvar!");

  switch ( m )
  {
    // Don't handle the on-diagonal case here
    // case 0: // density

  case 1:
  case 2:
  case 3:
  {
    // If jvar is one of the momentums, the derivative is a mass
    // matrix times that normal component...
    return _phi[_j][_qp] * _test[_i][_qp] * _normals[_qp](m-1);
  }

  
  case 4: // energy
    return 0.;

  default:
    mooseError("Should not get here!");
  }

  return 0.;
}
