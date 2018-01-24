/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSMassUnspecifiedNormalFlowBC.h"

template <>
InputParameters
validParams<NSMassUnspecifiedNormalFlowBC>()
{
  InputParameters params = validParams<NSMassBC>();
  params.addClassDescription("This class implements the mass equation boundary term with the "
                             "rho*(u.n) boundary integral computed implicitly.");
  return params;
}

NSMassUnspecifiedNormalFlowBC::NSMassUnspecifiedNormalFlowBC(const InputParameters & parameters)
  : NSMassBC(parameters)
{
}

Real
NSMassUnspecifiedNormalFlowBC::computeQpResidual()
{
  const RealVectorValue mom(_rho_u[_qp], _rho_v[_qp], _rho_w[_qp]);
  return qpResidualHelper(mom * _normals[_qp]);
}

Real
NSMassUnspecifiedNormalFlowBC::computeQpJacobian()
{
  return qpJacobianHelper(/*on diagonal=*/0);
}

Real
NSMassUnspecifiedNormalFlowBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (isNSVariable(jvar))
    return qpJacobianHelper(mapVarNumber(jvar));
  else
    return 0.0;
}
