/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSMassUnspecifiedNormalFlowBC.h"

template<>
InputParameters validParams<NSMassUnspecifiedNormalFlowBC>()
{
  InputParameters params = validParams<NSMassBC>();

  return params;
}



NSMassUnspecifiedNormalFlowBC::NSMassUnspecifiedNormalFlowBC(const InputParameters & parameters)
    : NSMassBC(parameters)
{
}





Real NSMassUnspecifiedNormalFlowBC::computeQpResidual()
{
  RealVectorValue mom(_rho_u[_qp], _rho_v[_qp], _rho_w[_qp]);

  return qpResidualHelper( mom * _normals[_qp] );
}




Real NSMassUnspecifiedNormalFlowBC::computeQpJacobian()
{
  return this->qp_jacobian( /*on diagonal=*/ 0);
}




Real NSMassUnspecifiedNormalFlowBC::computeQpOffDiagJacobian(unsigned jvar)
{
  return this->qp_jacobian( mapVarNumber(jvar) );
}

