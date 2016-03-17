/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSMassBC.h"

template<>
InputParameters validParams<NSMassBC>()
{
  InputParameters params = validParams<NSIntegratedBC>();


  return params;
}



NSMassBC::NSMassBC(const InputParameters & parameters)
    : NSIntegratedBC(parameters)
{
}




Real NSMassBC::qpResidualHelper(Real rhoun)
{
  return rhoun * _test[_i][_qp];
}




Real NSMassBC::qp_jacobian(unsigned var_number)
{
  switch ( var_number )
  {
  case 0: // density
  case 4: // energy
    return 0.;

  case 1:
  case 2:
  case 3: // momentums
  {
    // If one of the momentums, the derivative is a mass
    // matrix times that normal component...
    return _phi[_j][_qp] * _test[_i][_qp] * _normals[_qp](var_number-1);
  }

  default:
    mooseError("Should not get here!");
    break;
  }

  // won't get here...
  return 0;
}



// Real NSMassBC::computeQpResidual()
// {
//   // (rho*u.n) phi_i
//   RealVectorValue mom(_rho_u[_qp], _rho_v[_qp], _rho_w[_qp]);
//
//   return (mom*_normals[_qp]) * _test[_i][_qp];
// }
//
//
//
// Real NSMassBC::computeQpJacobian()
// {
//   // The derivative wrt rho is zero!
//   return 0.;
// }
//
//
//
// Real NSMassBC::computeQpOffDiagJacobian(unsigned jvar)
// {
//   // Map jvar into the variable m for our problem, regardless of
//   // how Moose has numbered things.
//   unsigned m = mapVarNumber(jvar);
//
//   switch ( m )
//   {
//     // Don't handle the on-diagonal case here
//     // case 0: // density
//
//   case 1:
//   case 2:
//   case 3:
//   {
//     // If jvar is one of the momentums, the derivative is a mass
//     // matrix times that normal component...
//     return _phi[_j][_qp] * _test[_i][_qp] * _normals[_qp](m-1);
//   }
//
//
//   case 4: // energy
//     return 0.;
//
//   default:
//     mooseError("Should not get here!");
//   }
//
//   // won't get here
//   return 0.;
// }

